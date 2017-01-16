#include "decoder.h"

#include <iostream>
#include <sstream>
#include <iomanip>

Decoder::Decoder() {
    ring_ = new RingBuffer();
    memory_state_ = new MemoryState();
    registerd_events = 0;

    setupEventDatabase();
}

void Decoder::setupEventDatabase() {
  int rc;
    rc = sqlite3_open("event.db", &event_db_);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(event_db_));
    }else{
      fprintf(stdout, "Opened event database successfully\n");
    }
    char *err_msg = 0;
    char *sql = "CREATE TABLE Events(EventNumber INTEGER PRIMARY KEY, Type TEXT, Heap INTEGER, Timestamp BIGINTEGER);";

    rc = sqlite3_exec(event_db_, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        
        return;
    }
}

RingBuffer* Decoder::getRingbuffer() {
    return ring_;
}

Decoder::~Decoder() {
    delete ring_;
    delete memory_state_;
    sqlite3_close(event_db_);
}

bool Decoder::decodeValue(uint64_t& ret) {
    uint64_t mul = 1;
    ret = 0;
    uint8_t b = 0;
    do {
        if(!ring_->readNext(b)) {
            ret = -1;
            return false;
        }
        ret = ret | (b*mul);
        mul = mul << 7;
    } while(b < 0x80);
    
    ret &= ~mul;
    return true;
}

bool Decoder::decodeValue(int& ret) {
  uint64_t val;
  bool decode = decodeValue(val);
  ret = (int)val;
  return decode;
}

bool Decoder::decodeString(std::string& ret) {

        uint64_t length;
        if(!decodeValue(length)) {
            return false;
        }
        if(!ring_->extractString(ret, length)) {
            return false;
        }
        return true;
}

void Decoder::trySteps() {
  //std::cout << "starting try steps\n";
  ring_->saveRollback();
  size_t current_count = registerd_events;
  size_t current_time = last_timestamp;
  size_t num_throwaway = 0;
std::stringstream temp;
  recording_ = false;
  do{
    ring_->saveRollback();
    bool success = oneStep(temp);
    
    if(!success) {
      ring_->loadRollback();
      uint8_t throwaway;
      ring_->readNext(throwaway);
      num_throwaway++;
    }
    else {
      break;
    }
  }while(ring_->getNumUnread() > 0);

  if(current_count + 1 != registerd_events && last_timestamp < current_time) {
    throw;
  }

  ring_->loadRollback(); //go back to right before successful event
  ring_->saveRollback(); //and save it

  recording_ = true;
  registerd_events = current_count;
  last_timestamp = current_time;
}

bool Decoder::oneStep(std::stringstream& sql) {
  int current_code;
  size_t count;

  if(!decodeValue(count)) {
    if(print_error) {std::cout << "\treading count failed" << std::endl;}
    return false;
  }

  if(count != registerd_events) {
    if(print_error) {std::cout << "\tgatherd event != registrd_events" << count << "!=" << registerd_events << std::endl;}
    return false;
  }

  if(!decodeValue(current_code)) {
    if(print_error) {std::cout << "\treading current_code failed" << std::endl;}
      return false;
  }

  size_t time_stamp;

  if(!decodeValue(time_stamp)) {
    if(print_error) {std::cout << "\treading time_stamp failed" << std::endl;}
      return false;
  }

  if( last_timestamp > time_stamp) {
    if(print_error) {std::cout << "\tlast_timestamp > current_timestamp: " << last_timestamp << " > " << time_stamp << std::endl;}
    return false;
  }

  switch(current_code) {
    case BeginStream :
    {
      std::string platform;
      size_t system_frequency;
      size_t stream_magic;

      if(!decodeValue(stream_magic)) {
        if(print_error) {std::cout << "\t\tDecode stream_magic failed" << std::endl;}
        return false;
      }
      if(!decodeString(platform)) {
        if(print_error) {std::cout << "\tDecode platform failed" << std::endl;}
        return false;
      }
      if(!decodeValue(system_frequency)) {
        if(print_error) {std::cout << "\tDecode system_frequency failed" << std::endl;}
        return false;
      }
      if(recording_) {
        sql << "INSERT INTO Events(EventNumber, Type, Timestamp)"
            " VALUES (" << count << ", " << current_code << ", " << time_stamp << ");";

      memory_state_->setInits(stream_magic, platform, system_frequency);
      
      }


      if(print_ok){std::cout << "(" << registerd_events << ")BeginStream\n\ttime_stamp: " << time_stamp << "\n\tplatform: " << platform << "\n\tsystem frequency: " << system_frequency << "\n";}
      break;
    }

    case EndStream :
      {
        if(print_ok){std::cout << "(" << registerd_events << ")Endstream\n\ttime_stamp: " << time_stamp << "\n\tDo nothing\n";}
        if(recording_) {
          sql << "INSERT INTO Events(EventNumber, Type, Timestamp)"
            " VALUES (" << count << ", " << current_code << ", " << time_stamp << ");";
        }
      }
    break;

    case HeapCreate :
    { 
      std::string name;
      int id;
      if(!decodeValue(id)) {
        if(print_error) {   std::cout << "decode Add Heap id failed\n";}
        return false;
      }
      if(!decodeString(name)) {
        if(print_error) {   std::cout << "decode Add Heap name failed\n";}
        return false;
      }
      if(recording_) {

       memory_state_->addHeap(id,name, time_stamp);

       sql << "INSERT INTO Events(EventNumber, Type, Timestamp)"
            " VALUES (" << count << ", " << current_code << ", " << time_stamp << ");";

      }
      if(print_ok){std::cout << "(" << registerd_events << ")HeapCreate\n\ttime_stamp: " << time_stamp << "\n\tId: " << id << "\n\tName: " << name << "\n";}
      break;
    }

    case HeapDestroy :
    {
      int id;
      if(!decodeValue(id)) {
        if(print_error) {   std::cout << "decode Heap Destroy id failed\n";}
        return false;
      }
      if(recording_) {
        memory_state_->removeHeap(id, time_stamp);

        sql << "INSERT INTO Events(EventNumber, Type, Timestamp)"
            " VALUES (" << count << ", " << current_code << ", " << time_stamp << ");";
      }
      if(print_ok){std::cout << "(" << registerd_events << ")HeapDestroy\n\ttime_stamp: " << time_stamp << "\n\tId: " << id << "\n";}
      break;
    }

    case HeapAddCore :
    {
      int id;
      size_t pointer;
      size_t size_bytes;
      if(!decodeValue(id)) {
        if(print_error) {   std::cout << "decode Add Core id failed\n";}
        return false;
      }
      if(!decodeValue(pointer)) {
        if(print_error) {  std::cout << "decode Add Core pointer failed\n";}
        return false;
      }
      if(!decodeValue(size_bytes)) {
        if(print_error) {  std::cout << "decode Add Core size failed\n";}
        return false;
      }
      if(recording_) {
      memory_state_->addCore(id,pointer,size_bytes,time_stamp);


      sql << "INSERT INTO Events(EventNumber, Type, Timestamp)"
            " VALUES (" << count << ", " << current_code << ", " << time_stamp << ");";

      }
       if(print_ok){std::cout << "(" << registerd_events << ")HeapAddCore\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; }
      break;
    }

    case HeapRemoveCore :
    {  
      int id;
      size_t pointer;
      size_t size_bytes;
      if(!decodeValue(id)) {
        if(print_error) {  std::cout << "decode Remove Core error failed\n";}
        return false;
      }
      if(!decodeValue(pointer)) {
        if(print_error) {  std::cout << "decode Remove Core pointer failed\n";}
        return false;
      }
      if(!decodeValue(size_bytes)) {
        if(print_error) { std::cout << "decode Remove Core size failed\n";}
        return false;
      }
      if(recording_) {
      memory_state_->removeCore(id,pointer,size_bytes, time_stamp);


      sql << "INSERT INTO Events(EventNumber, Type, Timestamp)"
            " VALUES (" << count << ", " << current_code << ", " << time_stamp << ");";

      }
       if(print_ok){std::cout << "(" << registerd_events << ")HeapRemoveCore\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; }
      break;
    }
    case HeapAllocate:
    {
      int id;
      size_t pointer;
      size_t size_bytes;
      if(!decodeValue(id)) {
        if(print_error) { std::cout << "decode Heap Allocate id failed\n";}
        return false;
      }
      if(!decodeValue(pointer)) {
        if(print_error) { std::cout << "decode Heap Allocate pointer failed\n";}
        return false;
      }
      if(!decodeValue(size_bytes)) {
        if(print_error) { std::cout << "decode Heap Allocate size failed\n";}
        return false;
      }
      if(recording_) {
      memory_state_->addAllocation(id,pointer,size_bytes, time_stamp);


      sql << "INSERT INTO Events(EventNumber, Type, Timestamp)"
            " VALUES (" << count << ", " << current_code << ", " << time_stamp << ");";
      }
       if(print_ok){std::cout << "(" << registerd_events << ")HeapAllocate\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n\tSize: " << size_bytes << "\n"; }
      break;
    }

    case HeapFree:
    { 
      int id;
      size_t pointer;
      if(!decodeValue(id)) {
        if(print_error) { std::cout << "decode HeapFree id failed\n";}
        return false;
      }
      if(!decodeValue(pointer)) {
        if(print_error) { std::cout << "decode HeapFree pointer failed\n";}
        return false;
      }
      if(recording_) {
      memory_state_->removeAllocation(id,pointer, time_stamp);



      sql << "INSERT INTO Events(EventNumber, Type, Timestamp)"
            " VALUES (" << count << ", " << current_code << ", " << time_stamp << ");";
        
      }
       if(print_ok){std::cout << "(" << registerd_events << ")HeapFree\n\ttime_stamp: " << time_stamp << "\n\tId: " << id <<"\n\tPointer: " << std::hex << pointer << std::dec << "\n"; }
      break;
    }
    default:
      std::cout << "Unhandled Event " << current_code << ", time_stamp: " << time_stamp << " num unread: " << ring_->getNumUnread() << "\n";
      return false;
    break;
  } //switch(current code)

  
  last_timestamp = time_stamp;
  registerd_events++;
  return true;
}

std::vector<Heap*> Decoder::getMemoryState() {
  return memory_state_->getHeaps();
}


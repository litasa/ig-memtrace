#include "heap.h"

#include <iostream>

Heap::Heap(int id,
    std::string name,
    size_t timestamp)
    : 
    MemoryObject(0, timestamp, 0, 0), 
    id_(id),
    name_(name)
    {
        
     }
Heap::~Heap() {
    for(auto it =cores_.begin(); it != cores_.end(); ++it) {
        delete it->second;
    }
}

void Heap::printContent() const {
    std::cout << "printing content for heap " << id_ << "\n";
    for(auto it = cores_.begin(); it != cores_.end(); ++it) {
        it->second->printContent();
    }
}

Core* Heap::getCore(size_t pointer) {
    auto found = cores_.find(pointer);
    if(found == cores_.end()) {
        Core* retval = nullptr;
        //std::cout << "Looking for core but could not find it: " << std::hex << pointer  << std::dec << std::endl;
        //std::cout << "is it recently_dead? ";
        for(auto it = recently_dead_.begin(); it != recently_dead_.end(); ++it) {
            if(it->second->getPointer() == pointer) {
                retval = (it->second);
            }
            // if(it->second.getNumberOfAllocations() == 0) {
            //     it = recently_dead_.erase(it);
            //     std::cout << "removed a recently dead core" << std::endl;
            // }
        }
        if(retval == nullptr) {
            //std::cout << " no\n";
        }
        else {
            //std::cout << " yes\n";
        }
        return retval;
    }
    return (found->second);
}

Core* Heap::getCoreForAllocation(size_t pointer) {
    auto found = alloc_to_core.find(pointer);
    if(found == alloc_to_core.end()) {
        std::cout << "No core mapper Found" << std::endl;
        return nullptr;
    }
    return getCore(found->second);
}

size_t Heap::removeCore(size_t pointer, size_t timestamp) {
    //std::cout << "\t\t\tCalling remove Core for: " << std::hex << pointer << std::dec << "at: "<< timestamp << "\n";
    Core* core = cores_.at(pointer);
    size_t managed = core->getManagedSize();
    if(core->getNumberOfAllocations() > 0) {
        Core* newCore = new Core(*core);
        core->death_ = timestamp;
        recently_dead_[pointer] = newCore;
        //std::cout << "adding core to recently dead";
    }
    
    size_t items_removed = cores_.erase(core->getPointer());
    delete core;
    
    if(items_removed == 1) {
        managed_memory_ -= managed;
        return managed;
    }
    return 0;
}

size_t Heap::addCore(size_t pointer, size_t timestamp, size_t managed_size) {
    Core* c = new Core(pointer,timestamp,managed_size);
    auto emp = cores_.insert(std::make_pair(pointer, c));
    if(!emp.second) {
        std::cout << "Adding Core failed pointer: " << std::hex << pointer << std::dec << " managed_size: " << managed_size << " timestamp: " << timestamp  << "\n";
        printContent();
        return 0;
    }
    managed_memory_ += managed_size;
    return managed_size;
}

size_t Heap::addAllocation(size_t pointer, size_t size, size_t timestamp) {
    for(auto it = cores_.begin(); it != cores_.end(); ++it) {
        if(it->second->addAllocation(pointer, size, timestamp)) {
            alloc_to_core.emplace(pointer,it->second->getPointer());
            used_memory_ += size;
            simple_allocation_events_.push_back(std::make_pair(timestamp, used_memory_));
            return size;
        }
    }
   // std::cout << "Did not find core for: " << std::hex << pointer << std::dec << " in heap: " << id_ << " ";
    //std::cout << "printing cores for allocator:\n";
    // for(auto it = cores_.begin(); it != cores_.end(); ++it) {
    //     std::cout << "pointer" << std::hex << it->second.pointer_ << " end: " << it->second.end_  << std::dec << " managed: " << it->second.managed_memory_ << " used: " << it->second.used_memory_;
    //     std::cout << "\n";
    // }
    return 0;
}

size_t Heap::removeAllocation(size_t pointer, size_t timestamp) {
    Core* core = getCoreForAllocation(pointer);
    if(core == nullptr) {
        return 0;
    }
    size_t removed_memory = core->removeAllocation(pointer,timestamp);
    used_memory_ -= removed_memory;
    simple_allocation_events_.push_back(std::make_pair(timestamp,used_memory_));
    return removed_memory;
}
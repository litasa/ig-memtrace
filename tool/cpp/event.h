#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <sstream>

namespace Event 
{
    enum Code
        {
            BeginStream     = 1,
            EndStream,

            HeapCreate = 10,
            HeapDestroy,

            HeapAddCore,
            HeapGrowCore,
            HeapRemoveCore,
            HeapShrinkCore,

            HeapAllocate,
            HeapFree,
            HeapFreeAll,

            EventStart = 20,
            EventEnd,

            /*Stingray specifics from here on*/
            SetBackingAllocator = 30,
        };

    struct Event {
        Event();
        Event(uint64_t eventNumber, size_t eventType, uint64_t timestamp)
        : eventNumber(eventNumber), eventType(eventType), timestamp(timestamp) {};
        ~Event() {};

        virtual void getAsCSV(std::stringstream& ss) {
		    ss << eventNumber << "," << eventType << "," << timestamp;
	    }

	    virtual void getAsVerbose(std::stringstream& ss) = 0;

        uint64_t eventNumber;
        size_t eventType;
        uint64_t timestamp;
    };

    struct RemoveAllAllocations : public Event {
        RemoveAllAllocations(uint64_t eventNumber, size_t eventType, uint64_t timestamp, uint64_t id)
        : Event(eventNumber, eventType, timestamp), id(id)
        { };
        ~RemoveAllAllocations();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "RemoveAllAllocations " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n";
        }
        uint64_t id;
    };

    struct InitStream : public Event {
        InitStream(uint64_t eventNumber, size_t eventType, size_t timestamp, size_t stream_magic, std::string& platform, size_t frequency)
        : Event(eventNumber, eventType, timestamp), stream_magic(stream_magic), platform(platform), system_frequency(frequency)
        { };
        ~InitStream();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << std::showbase << std::hex << stream_magic << std::dec << "," << platform << "," << system_frequency << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "InitStream " << "at time: " << timestamp;
            ss << "\n\tstream magic: " << std::showbase << std::hex << stream_magic << std::dec;
            ss << "\n\tplatform: " << platform;
            ss << "\n\tsystem frequency: " << system_frequency;
            ss << "\n";
        }

        uint64_t stream_magic;
        std::string platform;
        uint64_t system_frequency;
    };

    struct StopStream : public Event {
        StopStream(uint64_t eventNumber, size_t eventType, uint64_t timestamp)
        : Event(eventNumber, eventType, timestamp)
        { };
        ~StopStream();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
            ss << ", #stream ended\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "StopStream " << "at time: " << timestamp << "\n";
        }
    };

    struct AddHeap : public Event {
        AddHeap(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id, std::string& type, std::string& name)
        : Event(eventNumber, eventType, time)
        , id(id)
        , type(type)
        , name(name)
        {};
        ~AddHeap();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << type << "," << name << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "AddHeap " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\ttype: " << type;
            ss << "\n\tname: " << name;
            ss << "\n";
        }

        uint64_t id;
        std::string type;
        std::string name;
    };

    struct RemoveHeap : public Event {
        RemoveHeap(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id)
        : Event(eventNumber, eventType, time), id(id) {};
        ~RemoveHeap();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "RemoveHeap " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n";
        }
        uint64_t id;
    };

    struct AddCore : public Event {
        AddCore(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~AddCore();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << std::showbase << std::hex << pointer << std::dec << "," << size << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "AddCore " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\tsize: " << size;
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct GrowCore : public Event {
        GrowCore(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~GrowCore();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << std::showbase << std::hex << pointer << std::dec << "," << size << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "GrowCore " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\tsize: " << size;
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct RemoveCore : public Event {
        RemoveCore(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~RemoveCore();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << std::showbase << std::hex << pointer << std::dec << "," << size << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "RemoveCore " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\tsize: " << size;
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct ShrinkCore : public Event {
        ShrinkCore(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~ShrinkCore();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << std::showbase << std::hex << pointer << std::dec << "," << size << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "ShrinkCore " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\tsize: " << size;
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct AddAllocation : public Event {
        AddAllocation(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes, bool owned_by_heap)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes), owned(owned_by_heap) { };
        ~AddAllocation();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << std::showbase << std::hex << pointer << std::dec << "," << size << "," << owned <<"\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "AddAllocation " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\tsize: " << size;
            ss << "\n\towned by heap: " << owned? "true" : "false";
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
        bool owned;
    };

    struct RemoveAllocation : public Event {
        RemoveAllocation(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer, bool owned_by_heap)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), owned(owned_by_heap) { };
        ~RemoveAllocation();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << std::showbase << std::hex << pointer << std::dec << "," << owned << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "RemoveAllocation " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\towned by heap: " << owned? "true" : "false";            
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        bool owned;
    };

    struct StartEvent : public Event {
        StartEvent(uint64_t eventNumber, size_t eventType, uint64_t time, std::string& name)
        : Event(eventNumber, eventType, time), name(name) { };
        ~StartEvent();

        virtual void getAsCSV(std::stringstream& ss) override {
            Event::getAsCSV(ss);
            ss << "," << name << "\n";
        }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "StartEvent" << "at time: " << timestamp;
            ss << "\n\tname: " << name << "\n";
        }

        std::string name;
    };

    struct EndEvent : public Event {
        EndEvent(uint64_t eventNumber, size_t eventType, uint64_t time, std::string& name)
        : Event(eventNumber, eventType, time), name(name) { };
        ~EndEvent();

        virtual void getAsCSV(std::stringstream& ss) override {
            Event::getAsCSV(ss);
            ss << "," << name << "\n";
        }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "EndEvent" << "at time: " << timestamp;
            ss << "\n\tname: " << name << "\n";
        }

        std::string name;
    };

    struct HeapSetBackingAllocator : public Event {
        HeapSetBackingAllocator(uint64_t eventNumber, size_t eventType, uint64_t time, size_t for_heap, size_t set_to_heap)
        : Event(eventNumber, eventType, time), for_heap(for_heap), set_to_heap(set_to_heap) { };
        ~HeapSetBackingAllocator();

        virtual void getAsCSV(std::stringstream& ss) override {
            Event::getAsCSV(ss);
            ss << "," << for_heap << "," << set_to_heap << "\n";
        }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "HeapSetBackingAllocator" << "at time: " << timestamp;
            ss << "\n\tfor heap: " << for_heap;
            ss << "\n\tset to heap: " << set_to_heap << "\n";
        }

        size_t for_heap;
        size_t set_to_heap;
    };

    // struct TrackAllocation : public Event {
    //     TrackAllocation(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
    //     : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
    //     ~TrackAllocation();

    //     virtual void getAsCSV(std::stringstream& ss) override {
	// 	    Event::getAsCSV(ss);
	// 	    ss << "," << id << "," << pointer << "," << size << "\n";
	//     }

    //     virtual void getAsVerbose(std::stringstream& ss) override {
    //         ss << "(" << eventNumber << ")" << "TrackAllocation " << "at time: " << timestamp;
    //         ss << "\n\tid: " << id;
    //         ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
    //         ss << "\n\tsize: " << size;
    //         ss << "\n";
    //     }

    //     uint64_t id;
    //     uint64_t pointer;
    //     uint64_t size;
    // };

    // struct TrackFree : public Event {
    //     TrackFree(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer)
    //     : Event(eventNumber, eventType, time), id(id), pointer(pointer) { };
    //     ~TrackFree();

    //     virtual void getAsCSV(std::stringstream& ss) override {
	// 	    Event::getAsCSV(ss);
	// 	    ss << "," << id << "," << pointer << "\n";
	//     }

    //     virtual void getAsVerbose(std::stringstream& ss) override {
    //         ss << "(" << eventNumber << ")" << "TrackFree " << "at time: " << timestamp;
    //         ss << "\n\tid: " << id;
    //         ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
    //         ss << "\n";
    //     }

    //     uint64_t id;
    //     uint64_t pointer;
    // };
}
#endif //EVENT_H
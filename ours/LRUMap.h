#ifndef TABLE_H
#define TABLE_H

#include <iterator>
#include <unordered_map>
#include <list>
#include "../doctest.h"

// Really space inefficient implementation
template<typename Key, typename T>
class LRUMap {
    typedef std::list<Key> KeyTracker;
    typedef typename KeyTracker::iterator KeyIter;

private:
    uint32_t map_size;
    // most recent placed at the back
    KeyTracker key_tracker;
    // mapping key, to  pair<item, pointer to key's position in key_tracker>
    std::unordered_map<Key, std::pair<T, KeyIter>> map {};

    KeyIter touch(Key key);
    void evict_last();

public:
    LRUMap(uint32_t map_size) : map_size(map_size) {};

    void insert(Key key, T item);
    T& get(Key key);
};

template <typename Key, typename T>
inline void LRUMap<Key, T>::insert(Key key, T item)
{
    auto key_iter = touch(key);

    auto p = std::make_pair(item, key_iter);
    map[key] = p;

    while (key_tracker.size() > map_size) {
        evict_last();
    }
}

template <typename Key, typename T>
inline T& LRUMap<Key, T>::get(Key key) {
    if (map.find(key) == map.end()) {
        insert(key, T{});
    } else {
        touch(key);
    }

    return map[key].first;
}

template <typename Key, typename T>
inline typename LRUMap<Key, T>::KeyIter LRUMap<Key, T>::touch(Key key)
{
    auto iter = std::find(key_tracker.begin(), key_tracker.end(), key);

    if (iter == key_tracker.end()) {
        // if key didn't existed, register a new one at front
        // TODO: maybe let the callsites handle this?
        key_tracker.push_front(key);
        return key_tracker.begin();
    }
    
    if (iter != key_tracker.begin() ) {
        // move to the head of list
        key_tracker.splice(key_tracker.begin(), key_tracker, std::next(iter));
    }
    return iter;
}

template <typename Key, typename T>
inline void LRUMap<Key, T>::evict_last()
{
    auto key = key_tracker.back();
    key_tracker.pop_back();
    map.erase(key);
}

// TODO: These are all external API tests; I want to test the internals as well, 
//       also perhaps add internal APIs that doesn't touch() for validation
TEST_SUITE("LRUMap") {
    TEST_CASE("LRUMap won't exceed maxsize") {
        auto map = LRUMap<uint32_t, uint32_t>(3);
        map.insert(1,1);
        map.insert(2,2);
        map.insert(3,3);

        // overflow! must pop (1,1)
        map.insert(4,4);

    	CHECK(map.get(2) == 2);
    	CHECK(map.get(3) == 3);
    	CHECK(map.get(4) == 4);

        // after overflow trying to access again will 
        // return re-inited defaults
    	CHECK(map.get(1) == 0);
    	CHECK(map.get(2) == 2);
    	CHECK(map.get(3) == 3);

        // and after touching 1,2,3 above, 4 got overflowed out here as well
    	CHECK(map.get(4) == 0);
    }

    TEST_CASE("Insert works as upsert") {
        auto map = LRUMap<uint32_t, uint32_t>(3);
        map.insert(1,1);
    	CHECK(map.get(1) == 1);

        map.insert(1,2);
    	CHECK(map.get(1) == 2);

        map.insert(1,3);
    	CHECK(map.get(1) == 3);

        map.insert(1,4);
    	CHECK(map.get(1) == 4);
    }

    TEST_CASE("Getting stuff that isn't registered yet returns default") {
        auto map = LRUMap<uint32_t, uint32_t>(3);
    	CHECK(map.get(1) == 0);
    	CHECK(map.get(2) == 0);
    	CHECK(map.get(3) == 0);
    }
}

#endif //TABLE_H
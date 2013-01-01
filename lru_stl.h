//
// C++ Interface: lru_stl
//
// Description:
//
//
// Author: ARH,,, <arh@aspire-one>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef LRU_STL_H
#define LRU_STL_H

#include <map>
#include <list>
#include "assert.h"
#include "iostream"
#include "global.h"

using namespace std;



// Class providing fixed-size (by number of records)
// LRU-replacement cache of a function with signature
// V f(K)
template <typename K, typename V> class lru_stl
{
public:
// Key access history, most recent at back
	typedef list<K> key_tracker_type;
// Key to value and key history iterator
	typedef map
	< K, pair<V, typename key_tracker_type::iterator> > 	key_to_value_type;
// Constuctor specifies the cached function and
// the maximum number of records to be stored.
	lru_stl(
	        V(*f)(const K& , V),
	        size_t c
	) : _fn(f) , _capacity(c) {
		///ARH: Commented for single level cache implementation
//         assert ( _capacity!=0 );
	}
	// Obtain value of the cached function for k
	int operator()(const K& k  , V& value, uint32_t status) {
		assert(_capacity != 0);
		PRINT(cout << "Access key: " << k << " with value: " << value << endl;);
// Attempt to find existing record
		const typename key_to_value_type::iterator it	= _key_to_value.find(k);

		if(it == _key_to_value.end()) {
// We don’t have it:
			PRINT(cout << "Miss on key: " << k << " with value: " << value << endl;);
// Evaluate function and create new record
			const V v = _fn(k, value);

			///ARH: write buffer inserts new elements only on write miss
			if(status & WRITE) {
				status |=  insert(k, v);
				PRINT(cout << "Insert done on key: " << k << " with value: " << value << endl;);
			}

			return (status | MISS);
		} else {
			PRINT(cout << "Hit on key: " << k << " with value: " << value << endl;);
// We do have it. Before returning value,
// update access record by moving accessed
// key to back of list.
			_key_tracker.splice(
			        _key_tracker.end(),
			        _key_tracker,
			        (*it).second.second
			);
			(*it).second.second = _key_tracker.rbegin().base();
			return (status | HIT);
		}
	} //end operator ()


	unsigned long long int get_min_key() {
		return (_key_to_value.begin())->first;
	}

	unsigned long long int get_max_key() {
// 			std::map< K, std::pair<V,typename key_tracker_type::iterator> >::iterator it;
		return (_key_to_value.rbegin())->first;
	}
	void remove(const K& k) {
		PRINT(cout << "Removing key " << k << endl;);
// Assert method is never called when cache is empty
		assert(!_key_tracker.empty());
// Identify  key
		const typename key_to_value_type::iterator it
		= _key_to_value.find(k);
		assert(it != _key_to_value.end());
		PRINT(cout << "Remove value " << (it->second).first << endl;);
// Erase both elements to completely purge record
		_key_to_value.erase(it);
		_key_tracker.remove(k);
	}
private:

// Record a fresh key-value pair in the cache
	int insert(const K& k, const V& v) {
		PRINT(cout << "insert key " << k << " ,value " << v << endl;);
		int status = 0;
// Method is only called on cache misses
		assert(_key_to_value.find(k) == _key_to_value.end());

// Make space if necessary
		if(_key_to_value.size() == _capacity) {
			PRINT(cout << "Cache is Full " << _key_to_value.size() << " sectors" << endl;);
			evict();
			status = EVICT;
		}

// Record k as most-recently-used key
		typename key_tracker_type::iterator it
		= _key_tracker.insert(_key_tracker.end(), k);
// Create the key-value entry,
// linked to the usage record.
		_key_to_value.insert(make_pair(k, make_pair(v, it)));
// No need to check return,
// given previous assert.
// 			add_sram_entry(k,false);
		return status;
	}
// Purge the least-recently-used element in the cache
	void evict() {
// Assert method is never called when cache is empty
		assert(!_key_tracker.empty());
// Identify least recently used key
		const typename key_to_value_type::iterator it
		= _key_to_value.find(_key_tracker.front());
		assert(it != _key_to_value.end());
		PRINT(cout << "evicting victim key " << (*it).first << " ,value " << (*it).second.first << endl;);
		sram_evict(it->first);
// Erase both elements to completely purge record
		_key_to_value.erase(it);
		_key_tracker.pop_front();
	}
// The function to be cached
	V(*_fn)(const K& , V);
// Maximum number of key-value pairs to be retained
	const size_t _capacity;

// Key access history
	key_tracker_type _key_tracker;
// Key-to-value lookup
	key_to_value_type _key_to_value;
};

#endif //end lru_stl

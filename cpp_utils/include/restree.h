/*
 * restree.h
 *
 *  Created on: Jun 19, 2019
 *      Author: Blythe Hospelhorn
 */

#ifndef RESTREE_RESTREE_H_
#define RESTREE_RESTREE_H_

#include <vector>
#include <list>
#include <map>
#include <stdexcept>
#include <cstddef>
#include <filesystem> //Need C++ 17 !

#include "wr_cpp_utils.h"

using std::map;
using std::vector;
using std::list;
using std::filesystem::path;

namespace waffleoRai_Utils{

/*--- Resource Key ---*/

typedef struct WRCU_DLL_API ResourceKey{

public:
	u32 typeID = ~0; //Technically this is an enum
	u32 groupID = ~0;
	u64 instanceID = ~0ULL;

//Quick constructor
    ResourceKey(){};
	ResourceKey(u32 type, u32 group, u64 instance):typeID(type),groupID(group),instanceID(instance){};
	ResourceKey(const ResourceKey& other):typeID(other.typeID),groupID(other.groupID),instanceID(other.instanceID){};

//Operator overrides

    ResourceKey& operator=(const ResourceKey& other);
	bool operator==(const ResourceKey& other) const;
	bool operator!=(const ResourceKey& other) const;
	bool operator>=(const ResourceKey& other) const;
	bool operator>(const ResourceKey& other) const;
	bool operator<=(const ResourceKey& other) const;
	bool operator<(const ResourceKey& other) const;

} ResourceKey;

/*--- Resource Card ---*/

class WRCU_DLL_API ResourceCard {

public:
	ResourceKey key;
	const path* filepath = nullptr; //Reference should be to something in path table for mem management
	u64 offset = ~0ULL;
	size_t rawSize = ~0ULL; //In TS3 Top bit is always set. Why? No one knows. Will unset upon readin.
	size_t decompSize = ~0ULL;
	u16 misc_flags = 0;
	bool compressed = false;
	bool isValid = true;
	string name;

	ResourceCard() :key(), name("") {};
	ResourceCard(const ResourceKey& rkey, const path& path) :key(rkey), filepath(&path), name("") {};
	ResourceCard(const u32 type, const u32 group, const u64 instance, const path& path) :key(type, group, instance), filepath(&path) {};
	ResourceCard(const ResourceCard& other) :key(other.key), filepath(other.filepath), offset(other.offset), rawSize(other.rawSize), decompSize(other.decompSize), compressed(other.compressed) {};

	ResourceCard& operator=(const ResourceCard& other);
	bool operator==(const ResourceCard& other) const;
	bool operator!=(const ResourceCard& other) const;
	bool operator>=(const ResourceCard& other) const;
	bool operator>(const ResourceCard& other) const;
	bool operator<=(const ResourceCard& other) const;
	bool operator<(const ResourceCard& other) const;

	virtual ~ResourceCard() {}
};

/*--- Resource Map ---*/

typedef std::map<ResourceKey, ResourceCard>::const_iterator ResMapItr;

class WRCU_DLL_API NoResourceCardException:public exception
{
private:
	const char* sSource;
	const char* sReason;
	const ResourceKey badkey;

public:
	NoResourceCardException(const char* source, const char* reason, ResourceKey& key):sSource(source),sReason(reason),badkey(key){};
	const char* what() const throw(){return sReason;}
	const char* getSource() const {return sSource;}
	const char* getReason() const {return sReason;}
	const ResourceKey& getBadKey() const{return badkey;}

};

class WRCU_DLL_API PathTable{

private:
	vector<path> str_vec;

	void init_core(const size_t init_alloc);

public:
	PathTable() :str_vec() { init_core(4); }
	PathTable(const size_t init_alloc) :str_vec(){ init_core(init_alloc); }
	void clear() { str_vec.clear(); }
	const size_t getSize() { return str_vec.size(); }
	const path& getPathAtIndex(const int idx);

	const int addPath(const path& p);
	const int findPath(const path& p);

	void realloc(const size_t alloc_sz) {
		str_vec.reserve(alloc_sz);
	}

};

class WRCU_DLL_API ResourceMap{

private:
	map<ResourceKey, ResourceCard> rMap;

public:
	ResourceMap():rMap(){};
	ResourceCard& getCard(u32 type, u32 group, u64 instance);
	ResourceCard& getCard(ResourceKey& key);

	const int getAllCardsOfType(u32 type, list<ResourceCard>& target) const;
	const int getAllCardsInGroup(u32 type, u32 group, list<ResourceCard>& target) const;
	const int getAllKeys(list<ResourceKey>& target) const;
	const int getAllCards(list<const ResourceCard*>& target) const;

	const int countCards() const;

	ResMapItr cbegin() const { return rMap.cbegin(); };
	ResMapItr cend() const { return rMap.cend(); };
	map<ResourceKey, ResourceCard>& getMapView() { return rMap; };

	const bool hasCard(const ResourceKey& key) const;
	ResourceCard* addCard(const ResourceKey& key);
	ResourceCard* addCard(const ResourceCard& card, const bool allow_overwrite);
	const bool removeCard(const ResourceKey& key);
	void clearCards();

	const int copyIntoMap(ResourceMap& target, list<ResourceCard*>& output_list) const;

};

}

#endif // RESTREE_RESTREE_H_ 

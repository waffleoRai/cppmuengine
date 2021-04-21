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

#include "wr_cpp_utils.h"

//using namespace std;
using std::map;
using std::vector;
using std::list;
using namespace icu;

namespace waffleoRai_Utils{

/*--- Resource Key ---*/

typedef struct WRCU_DLL_API ResourceKey{

public:
	u32 typeID = ~0; //Technically this is an enum
	u32 groupID = ~0;
	u64 instanceID = ~0;

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
	const UnicodeString* pathroot = nullptr;
	const string* filepath = nullptr; //Reference should be to something in path table for mem management
	u64 offset = ~0ULL;
	size_t rawSize = ~0ULL; //In TS3 Top bit is always set. Why? No one knows. Will unset upon readin.
	size_t decompSize = ~0ULL;
	u16 misc_flags = 0;
	bool compressed = false;
	string name;

	ResourceCard() :key(), name("") {};
	ResourceCard(const ResourceKey& rkey, string& path) :key(rkey), filepath(&path), name("") {};
	ResourceCard(const u32 type, const u32 group, const u64 instance, string& path) :key(type, group, instance), filepath(&path) {};
	ResourceCard(const ResourceCard& other) :key(other.key), filepath(other.filepath), offset(other.offset), rawSize(other.rawSize), decompSize(other.decompSize), compressed(other.compressed) {};

	ResourceCard& operator=(const ResourceCard& other);
	bool operator==(const ResourceCard& other) const { return key == other.key; }
	bool operator!=(const ResourceCard& other) const { return key != other.key; }
	bool operator>=(const ResourceCard& other) const { return key >= other.key; }
	bool operator>(const ResourceCard& other) const { return key > other.key; }
	bool operator<=(const ResourceCard& other) const { return key <= other.key; }
	bool operator<(const ResourceCard& other) const { return key < other.key; }

	virtual const size_t pathLength();
	virtual const size_t getUnicodePath(char16_t* dst, size_t dstcap);

	virtual ~ResourceCard() {}
};

/*--- Resource Map ---*/

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
	//TODO switch to vector?
private:
    //list<string> str_list;
	UnicodeString basepath = "";
	vector<string> str_vec;

public:
    /*PathTable():str_list(){};
    const string& addPath(const string& strpath){str_list.push_back(strpath); return str_list.back();}
    void clear(){str_list.clear();}
    const size_t getSize(){return str_list.size();}*/

	PathTable(size_t init_alloc) :str_vec(init_alloc){}
	void clear() { str_vec.clear(); }
	const size_t getSize() { return str_vec.size(); }
	const string& getPathAtIndex(const int idx);
	const UnicodeString& getBasePath();

	const string& addPath(const string& strpath);
	const string& addPath(const char* path);
	const UnicodeString& setBasePath(const UnicodeString& in);

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

	const bool hasCard(const ResourceKey& key) const;
	ResourceCard* addCard(const ResourceCard& card);
	const bool removeCard(const ResourceKey& key);
	void clearCards();

	const int copyIntoMap(ResourceMap& target, list<ResourceCard*>& output_list) const;

};

}

#endif // RESTREE_RESTREE_H_ 

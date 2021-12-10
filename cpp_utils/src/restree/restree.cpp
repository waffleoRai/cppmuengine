/*
 * restree.cpp
 *
 *  Created on: Jun 23, 2019
 *      Author: Blythe Hospelhorn
 */

#include "restree.h"

using namespace std;

namespace waffleoRai_Utils{

/*--- Resource Key ---*/

ResourceKey& ResourceKey::operator=(const ResourceKey& other){
    if(this != &other){
        instanceID = other.instanceID;
        groupID = other.groupID;
        typeID = other.typeID;
    }
    return *this;
}

bool ResourceKey::operator==(const ResourceKey& other) const{
	if(instanceID != other.instanceID) return false;
	if(groupID != other.groupID) return false;
	if(typeID != other.typeID) return false;
	return true;
}

bool ResourceKey::operator!=(const ResourceKey& other) const{
	if(instanceID != other.instanceID) return true;
	if(groupID != other.groupID) return true;
	if(typeID != other.typeID) return true;
	return false;
}

bool ResourceKey::operator>=(const ResourceKey& other) const{
	if (typeID != other.typeID) {
		if (typeID < other.typeID) return false;
		else return true;
	}
	if (groupID != other.groupID) {
		if (groupID < other.groupID) return false;
		else return true;
	}
	if (instanceID < other.instanceID) return false;
	return true;
}

bool ResourceKey::operator<=(const ResourceKey& other) const{
	if (typeID != other.typeID) {
		if (typeID > other.typeID) return false;
		else return true;
	}
	if (groupID != other.groupID) {
		if (groupID > other.groupID) return false;
		else return true;
	}
	if (instanceID > other.instanceID) return false;
	return true;
}

bool ResourceKey::operator>(const ResourceKey& other) const{
	//I guess this needs to be more thorough for MSVC...
	if(typeID > other.typeID) return true;
	if (typeID < other.typeID) return false;

	if(groupID > other.groupID) return true;
	if (groupID < other.groupID) return false;

	if(instanceID > other.instanceID) return true;
	return false;
}

bool ResourceKey::operator<(const ResourceKey& other) const{
	//I guess this needs to be more thorough for MSVC...
	if(typeID < other.typeID) return true;
	if (typeID > other.typeID) return false;

	if(groupID < other.groupID) return true;
	if (groupID > other.groupID) return false;

	if(instanceID < other.instanceID) return true;
	return false;
}

/*--- Resource Card ---*/

ResourceCard& ResourceCard::operator=(const ResourceCard& other){
    if(this != &other){
        key = other.key;
        filepath = other.filepath;
        offset = other.offset;
        rawSize = other.rawSize;
        decompSize = other.decompSize;
        compressed = other.compressed;
        name = other.name;
    }
    return *this;
}

const size_t ResourceCard::pathLength() {
	size_t len = 0;
	
	if (pathroot) len += static_cast<size_t>(pathroot->length()) + 1;
	if (filepath) len += filepath->length();

	return len;
}

const size_t ResourceCard::getUnicodePath(char16_t* dst, size_t dstcap) {
	if (!dst || dstcap < 1) return 0;

	char16_t* lim = (dst + dstcap) - 1; 
	size_t count = 0;
	size_t strlen = 0;
	size_t i = 0;
	if (pathroot) {
		//Copy pathroot and filesep
		strlen = static_cast<size_t>(pathroot->length());
		for (i = 0; i < strlen; i++) {
			if (dst >= lim) break;
			*(dst++) = pathroot->charAt(i);
			count++;
		}
		if (dst < lim) { *(dst++) = FILE_SEP16; count++; }
	}

	if ((dst < lim) && filepath) {
		strlen = static_cast<size_t>(filepath->length());
		for (i = 0; i < strlen; i++) {
			if (dst >= lim) break;
			*(dst++) = filepath->at(i);
			count++;
		}
	}

	*(dst) = '\0';
	return count;
}

bool ResourceCard::operator==(const ResourceCard& other) const { 
	return key == other.key; 
}

bool ResourceCard::operator!=(const ResourceCard& other) const { 
	return key != other.key; 
}

bool ResourceCard::operator>=(const ResourceCard& other) const { 
	return key >= other.key; 
}

bool ResourceCard::operator>(const ResourceCard& other) const {
	return key > other.key; 
}

bool ResourceCard::operator<=(const ResourceCard& other) const {
	return key <= other.key; 
}

bool ResourceCard::operator<(const ResourceCard& other) const {
	return key < other.key; 
}

/*--- Path Table ---*/

const string& PathTable::getPathAtIndex(const int idx) {
	if (idx < 0 || idx >= str_vec.size()) throw IndexOutOfBoundsException("waffleoRai_Utils::PathTable::getPathAtIndex","Index is invalid!");
	return str_vec[idx];
}

const string& PathTable::addPath(const string& strpath) {
	str_vec.push_back(strpath);
	return str_vec.back();
}

const string& PathTable::addPath(const char* path) {
	str_vec.push_back(path);
	return str_vec.back();
}

const UnicodeString& PathTable::getBasePath() {
	return basepath;
}

const UnicodeString& PathTable::setBasePath(const UnicodeString& in) {
	basepath = in;
	return basepath;
}

/*--- Resource Map ---*/

ResourceCard& ResourceMap::getCard(u32 type, u32 group, u64 instance){
	ResourceKey key = ResourceKey(type, group, instance);
	return getCard(key);
}

ResourceCard& ResourceMap::getCard(ResourceKey& key){
	try{
		return rMap.at(key);
	}
	catch(out_of_range& ex){
		throw NoResourceCardException("waffleoRai_Utils::ResourceMap::getCard", "Resource card with requested key not in map!", key);
	}
}

const int ResourceMap::getAllCardsOfType(u32 type, list<ResourceCard>& target) const{
	//TODO Copies to list?? Or referenced by list?
	//map<ResourceKey, ResourceCard>::iterator itr;
	int lcount = 0;
	for (map<ResourceKey, ResourceCard>::const_iterator itr = rMap.begin(); itr != rMap.end(); itr++)
	{
		const ResourceKey& k = itr->first;
		if(k.typeID == type)
		{
			target.push_back(itr->second);
			lcount++;
		}
	}

	return lcount;
}

const int ResourceMap::getAllCardsInGroup(u32 type, u32 group, list<ResourceCard>& target) const{
	//TODO Copies to list?? Or referenced by list?
	//map<ResourceKey, ResourceCard>::iterator itr;
	int lcount = 0;
	for (map<ResourceKey, ResourceCard>::const_iterator itr = rMap.begin(); itr != rMap.end(); itr++)
	{
		const ResourceKey& k = itr->first;
		if((k.typeID == type) && (k.groupID == group))
		{
			target.push_back(itr->second);
			lcount++;
		}
	}

	return lcount;
}

const int ResourceMap::getAllKeys(list<ResourceKey>& target) const{
	map<ResourceKey, ResourceCard>::const_iterator itr;
	int lcount = 0;
	for (itr = rMap.begin(); itr != rMap.end(); itr++)
	{
		const ResourceKey& k = itr->first;
		target.push_back(k);
		lcount++;
	}

	return lcount;
}

const int ResourceMap::getAllCards(list<const ResourceCard*>& target) const{
    map<ResourceKey, ResourceCard>::const_iterator itr;
	int lcount = 0;
	for (itr = rMap.begin(); itr != rMap.end(); itr++)
	{
		const ResourceCard* cptr = &(itr->second);
		target.push_back(cptr);
		lcount++;
	}

	return lcount;
}

const int ResourceMap::countCards() const{
	return rMap.size();
}

const bool ResourceMap::hasCard(const ResourceKey& key) const{
	//printf("KEY: %08x:%08x:%016llx\n", key.typeID, key.groupID, key.instanceID);
	map<ResourceKey, ResourceCard>::const_iterator itr = rMap.find(key); //There is some issue with this - "invalid comparator???"
	return(itr != rMap.end());
}

ResourceCard* ResourceMap::addCard(const ResourceCard& card){
	if(hasCard(card.key)) return nullptr;
	rMap[card.key] = card;
	return &(rMap[card.key]);
}

const bool ResourceMap::removeCard(const ResourceKey& key){
	size_t i = rMap.erase(key);
	return (i == 1);
}

void ResourceMap::clearCards(){
	rMap.clear();
}

const int ResourceMap::copyIntoMap(ResourceMap& target, list<ResourceCard*>& output_list) const{
    int ct = 0;
    map<ResourceKey, ResourceCard>::const_iterator itr;
	for (itr = rMap.begin(); itr != rMap.end(); itr++){
		//okay = okay && target.addCard(itr->second);
		ResourceCard* acard = target.addCard(itr->second);
		if(acard != nullptr){
          output_list.push_back(acard);
          ct++;
		}
	}
	return ct;
}

}

/*
 * Copyright (c) 2006 Apple Computer, Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

//
// resource directory construction and verification
//
#ifndef _H_RSIGN
#define _H_RSIGN

#include "CodeSigner.h"
#include "renum.h"
#include <security_utilities/utilities.h>
#include "regex.h"
#include <vector>

namespace Security {
namespace CodeSigning {


//
// The builder of ResourceDirectories.
//
// Note that this *is* a ResourceEnumerate, which can enumerate
// its source directory once (only).
//
class ResourceBuilder : public ResourceEnumerator {
public:
	ResourceBuilder(const std::string &root, CFDictionaryRef rules);
	~ResourceBuilder();

	CFDictionaryRef build();

	enum Action {
		optional = 0x01,				// may be absent at runtime
		omitted = 0x02,					// do not seal even if present
	};
	
	typedef unsigned int Weight;
	
public:
	class Rule : private regex_t {
	public:
		Rule(const std::string &pattern, Weight weight, uint32_t flags);
		~Rule();
		
		bool match(const char *s) const;
		
		const Weight weight;
		const uint32_t flags;
	};
	void addRule(Rule *rule) { mRules.push_back(rule); }
	
	FTSENT *next(std::string &path, Rule * &rule);	// enumerate next file and match rule

protected:
	void addRule(CFTypeRef key, CFTypeRef value);
	CFDataRef hashFile(const char *path);
	
private:
	CFCopyRef<CFDictionaryRef> mRawRules;
	typedef std::vector<Rule *> Rules;
	Rules mRules;
};


//
// The "seal" on a single resource.
//
class ResourceSeal {
public:
	ResourceSeal(CFTypeRef ref);

public:
	operator bool () const { return mHash; }
	bool operator ! () const { return mHash == NULL; }
	
	const SHA1::Byte *hash() const { return CFDataGetBytePtr(mHash); }
	bool optional() const { return mOptional; }

private:
	CFDataRef mHash;
	int mOptional;
};


} // end namespace CodeSigning
} // end namespace Security

#endif // !_H_RSIGN
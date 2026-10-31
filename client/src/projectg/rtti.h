#pragma once

class WRTTI
{
public:
	WRTTI(const char* name, const WRTTI* baseRTTI);
	const char* GetName() const { return m_pName; }
	const WRTTI* GetBaseRTTI() const { return m_pBaseRTTI; }

protected:
	const char* m_pName;
	const WRTTI* m_pBaseRTTI;
};

static __declspec(thread) void* __rtti_obj;

#define DYNAMIC_CAST(type, obj) \
	((__rtti_obj = (void*)(obj)) \
			? (type*)((IObject*)__rtti_obj)->DynamicCast(&type::m_RTTI) \
			: NULL)

#define IS_KINDOF(type, obj) \
	((__rtti_obj = (void*)(obj)) \
			? ((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) \
			: false)

#define IS_EXACTKINDOF(type, obj) \
	((__rtti_obj = (void*)(obj)) \
			? ((IObject*)__rtti_obj)->IsExactKindOf(&type::m_RTTI) \
			: false)

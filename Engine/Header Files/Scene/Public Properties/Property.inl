#include "Property.h"

template<typename PropertyType>
inline Property<PropertyType>::Property(std::string propertyName, std::shared_ptr<PropertyType> dataPtr) :
    m_propertyName(propertyName), m_dataPtr(dataPtr)
{

}

template<class PropertyType>
inline Property<PropertyType>::Property(std::string propertyName, std::shared_ptr<PropertyType> dataPtr, const PropertyType& minimum, const PropertyType& maximum) :
    m_propertyName(propertyName), m_dataPtr(dataPtr), m_minimum(minimum), m_maximum(maximum)
{
    EngineAssert(m_minimum > m_maximum);
}

template<class PropertyType>
inline Property<PropertyType>::Property(std::string propertyName, CustomPropertySetter customSetter, CustomPropertyGetter customGetter) :
    m_propertyName(propertyName), SetValue(customSetter), GetValue(customGetter)
{

}

template<class PropertyType>
inline Property<PropertyType>::Property(std::string propertyName, CustomPropertySetter customSetter, CustomPropertyGetter customGetter, const PropertyType& minimum, const PropertyType& maximum) :
m_propertyName(propertyName), SetValue(customSetter), GetValue(customGetter), m_minimum(minimum), m_maximum(maximum)
{
    EngineAssert(m_minimum > m_maximum);
}

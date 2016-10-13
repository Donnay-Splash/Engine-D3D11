#pragma once
#include "Property.h"
#include <vector>
#include <map>
// Any classes derived from this can be represented in the debug UI as part of a 
// SceneNode or Component. This class contains functions to add Property's that can be
// accessed by the UI layer, It also has a name that should be defined by the inherited class
// to be displayed in the UI 
class SceneElement : public std::enable_shared_from_this<SceneElement>
{
public:
    SceneElement(std::string elementName);

    Property::Ptr GetProperty(std::string name);
    void AddProperty(Property::Ptr property);

    // TODO: add functions to set and get values to avoid returning pointers.

    // Used when constructing UI. Not advisable to use otherwise
    std::vector<Property::Ptr> GetProperties();
    std::string GetElementName()const { return m_name; }
private:
    std::map<std::string, Property::Ptr> m_publicProperties;
    std::string m_name;
};
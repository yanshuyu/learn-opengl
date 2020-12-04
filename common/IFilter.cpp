#include"IFilter.h"


RTTI_IMPLEMENTATION(FilterComponent)


FilterComponent::FilterComponent(const std::string& name) : RenderableComponent()
, m_name(name)
, m_params() {

}


IFilter::IFilter(const std::string& name, PostProcessingManager* mgr) : m_name(name)
, m_manager(mgr) {

}
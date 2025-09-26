#include "RenderList.h"

#include <algorithm>

RenderList::RenderList()
{
    _reallocate(4);
}

RenderList::~RenderList()
{
    delete[] m_RenderablePrimitives;
    m_RenderablePrimitives = nullptr;
}

void RenderList::Draw()
{
    for (size_t i=0; i<m_PrimitiveCount; ++i)
    {
        m_RenderablePrimitives[i]->Bind();
        m_RenderablePrimitives[i]->Dispatch();
        m_RenderablePrimitives[i]->Unbind();
    }
}

void RenderList::_reallocate(const size_t new_capacity)
{
    IRenderable** new_data = new IRenderable*[new_capacity + sizeof(IRenderable)];
    if (new_capacity < m_PrimitiveCount)
    {
        this->m_PrimitiveCount = new_capacity;
    }

    for (size_t i = 0; i < this->m_PrimitiveCount; ++i)
    {
        new(&*new_data[i]) IRenderable*(std::move(m_RenderablePrimitives[i]));
    }
    for (size_t i = 0; i < this->m_PrimitiveCount; ++i)
    {
        this->m_RenderablePrimitives[i]->~IRenderable();
    }
    delete[] m_RenderablePrimitives;
    m_RenderablePrimitives = new_data;
    this->m_PrimitiveCapacity = new_capacity;
}

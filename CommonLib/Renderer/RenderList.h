#ifndef RENDER_LIST_H
#define RENDER_LIST_H

#include <utility>

#include "IRenderable.h"

class RenderList
{
public:
    RenderList();
    ~RenderList();
    
public:
    template<typename PrimitiveT>
    void Append(PrimitiveT* renderable);
    
    template<typename PrimitiveT>
    void Append(PrimitiveT& renderable);

    template<typename PrimitiveT>
    void Append(PrimitiveT&& renderable);
    
    void Draw();

private:
    void _reallocate(const size_t new_capacity);

    template<typename PrimitiveT>
    void _reallocateTyped(const size_t new_capacity);
    
private:
    IRenderable **m_RenderablePrimitives = nullptr;
    size_t m_PrimitiveCount = 0;
    size_t m_PrimitiveCapacity = 0;
};

template <typename PrimitiveT>
void RenderList::Append(PrimitiveT* renderable)
{
    if (renderable == nullptr) {return;}
    if (m_PrimitiveCount >= m_PrimitiveCapacity)
    {
        _reallocateTyped<PrimitiveT>(m_PrimitiveCapacity * 2);
    }
    m_RenderablePrimitives[m_PrimitiveCount] = renderable;
    m_PrimitiveCount++;
}

template <typename PrimitiveT>
void RenderList::Append(PrimitiveT& renderable)
{
    if (m_PrimitiveCount >= m_PrimitiveCapacity)
    {
        _reallocateTyped<PrimitiveT>(m_PrimitiveCapacity * 2);
    }
    m_RenderablePrimitives[m_PrimitiveCount] = &renderable;
    m_PrimitiveCount++;
}

template <typename PrimitiveT>
void RenderList::Append(PrimitiveT&& renderable)
{
    if (m_PrimitiveCount >= m_PrimitiveCapacity)
    {
        _reallocateTyped<PrimitiveT>(m_PrimitiveCapacity);
    }

    m_RenderablePrimitives[m_PrimitiveCount] = new PrimitiveT(std::forward<PrimitiveT>(renderable));
    m_PrimitiveCount++;
}

template <class PrimitiveT>
void RenderList::_reallocateTyped(const size_t new_capacity)
{
    IRenderable** new_data = new IRenderable*[new_capacity + sizeof(IRenderable)];
    if (new_capacity < m_PrimitiveCount)
    {
        this->m_PrimitiveCount = new_capacity;
    }

    for (size_t i = 0; i < this->m_PrimitiveCount; ++i)
    {
        new_data[i] = static_cast<PrimitiveT*>(m_RenderablePrimitives[i]);
    }
    delete[] m_RenderablePrimitives;
    m_RenderablePrimitives = new_data;
    this->m_PrimitiveCapacity = new_capacity;
}

#endif

#pragma once 

#include "engine/Types.h"

namespace fz
{
    // https://zeux.io/2010/10/17/aabb-from-obb-with-component-wise-abs/
    // https://madmann91.github.io/2024/02/10/converting-oriented-bounding-boxes-to-axis-aligned-ones.html
    struct AABB
    {
        Toad::Vec2f min;
        Toad::Vec2f max;
    };

    struct Node
    {   
        AABB data;
        size_t children[4];
    };
    struct Tree
    {

    };
}
#pragma once

#include <memory>
#include <vector>
#include "../MessageSystem/Component.h"

class GameObject {

    public:
        std::string name = "Default";
        std::vector<std::shared_ptr<Component>> components;

        GameObject() = default;

        template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr>
        GameObject(std::shared_ptr<T> component) {
            addComponent(component);
        }


        template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr>
        void addComponent(std::shared_ptr<T> component) {
            components.push_back(component);
        }

};
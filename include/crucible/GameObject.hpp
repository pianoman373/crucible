#pragma once

#include <crucible/Model.hpp>
#include <vector>

class Scene;
class RigidBody;

class GameObject;

class Component {
private:
    GameObject *parent;

public:
    void setParent(GameObject *parent);

    GameObject *getParent();

    virtual void render();

    virtual void update(float delta);
};

class ModelComponent : public Component {
private:
    Mesh *mesh;
    Material *material;

public:
    ModelComponent(Mesh *mesh, Material *material);

    void render();
};

class GameObject {
private:
    std::vector<Component*> components;
    std::string name;
    Scene *scene;
    RigidBody *rb;

public:
    Transform transform;

    ~GameObject();

    GameObject(Scene *scene, Transform transform, std::string name);

    void render();

    void update(float delta);

    RigidBody *addRigidBody(float mass);

    RigidBody *getRigidBody();

    void addComponent(Component *c);

    int getNumComponents();

    Component *getComponent(int index);

    std::string getName();

    void setName(std::string name);

    template <typename T>
    T *getComponent() {
        for (int i = 0; i < components.size(); i++) {
            T *ptr = dynamic_cast<T*>(components[i]);

            if (ptr) {
                return ptr;
            }
        }
        return nullptr;
    }

    template <typename T>
    std::vector<T*> getComponents() {
        std::vector<T*> ret;

        for (int i = 0; i < components.size(); i++) {
            T *ptr = dynamic_cast<T*>(components[i]);

            if (ptr) {
                ret.push_back(ptr);
            }
        }

        return ret;
    }
};


#include <crucible/GameObject.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/RigidBody.hpp>

#include <iostream>

void Component::setParent(GameObject *parent) {
    this->parent = parent;
}

GameObject *Component::getParent() {
    return this->parent;
}

void Component::render() {

}

void Component::update(float delta) {

}

ModelComponent::ModelComponent(Mesh *mesh, Material *material) {
    this->mesh = mesh;
    this->material = material;
}

void ModelComponent::render() {
    Renderer::render(mesh, material, this->getParent()->transform, AABB());
}

GameObject::~GameObject() {
    for (int i = 0; i < components.size(); i++) {
        delete components[i];
    }
    components.clear();

    if (this->rb) {
        delete this->rb;
    }
}

GameObject::GameObject(Scene *scene, Transform transform, std::string name) {
    this->transform = transform;
    this->name = name;
    this->scene = scene;
    this->rb = nullptr;

}

RigidBody *GameObject::addRigidBody(float mass) {
    this->rb = new RigidBody(this->scene, this->transform, mass);

    return this->rb;
}

RigidBody *GameObject::getRigidBody() {
    return this->rb;
}

void GameObject::addComponent(Component *c) {
    this->components.push_back(c);
    c->setParent(this);
}

int GameObject::getNumComponents() {
    return components.size();
}

Component *GameObject::getComponent(int index) {
    return components.at(index);
}

void GameObject::render() {
    for (int i = 0; i < components.size(); i++) {
        components[i]->render();
    }
}

void GameObject::update(float delta) {
    if (rb) {
        transform.position = rb->getPosition();
        transform.rotation = rb->getRotation();
    }


    for (int i = 0; i < components.size(); i++) {
        components[i]->update(delta);
    }
}

std::string GameObject::getName() {
    return this->name;
}

void GameObject::setName(std::string name) {
    this->name = name;
}
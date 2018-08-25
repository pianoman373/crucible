#include <crucible/GameObject.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/RigidBody.hpp>

#include <iostream>

void Component::setParent(GameObject *parent) {
    this->parent = parent;
}

GameObject *Component::getParent() const {
    return this->parent;
}

void Component::render() {

}

void Component::update(float delta) {

}

ModelComponent::ModelComponent(const Mesh &mesh, const Material &material): mesh(mesh), material(material) {

}

void ModelComponent::render() {
    Renderer::render(mesh, material, this->getParent()->transform);
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

GameObject::GameObject(Scene &scene, const Transform &transform, const std::string &name): scene(scene) {
    this->transform = transform;
    this->name = name;
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

const std::string &GameObject::getName() const {
    return this->name;
}

void GameObject::setName(const std::string &name) {
    this->name = name;
}
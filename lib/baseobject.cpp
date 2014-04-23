#include "baseobject.h"

#include <ctime>
#ifdef __GNUC__
#include <typeinfo>
#include <cxxabi.h>
#endif
#include <stdio.h>
#include <vector>

BaseObject::BaseObject(BaseObject *parent)
{
    _hasAParent = false;
    _parent = NULL;
    _inheritsRandom = false;
    _wasCloned = false;
    setParent(parent);
}

BaseObject::~BaseObject() {
    if(!_hasAParent) {
        deleteAllChildren();
    }
    else
        _parent->removeChild(this);
}

/**
 * @brief Adds the given child to the child list
 * @param child
 */

void BaseObject::addChild(BaseObject *child) {
    _childList.push_back(child);
    addChild_impl(child);
}

void BaseObject::addChild_impl(BaseObject *child) {

}

/**
 * @brief Removes the given child from the child list
 * @param child
 */

void BaseObject::removeChild(BaseObject *child) {
    _childList.remove(child);
    removeChild_impl(child);
}

void BaseObject::removeChild_impl(BaseObject *child) {

}

BaseObject *BaseObject::clone_impl() const
{
    return new BaseObject();
}

void BaseObject::deleteAllChildren() {
    BaseObject *child;
    while(!_childList.empty()) {
        child = _childList.back();
        _childList.remove(child);
        delete child;
    }
}

list<BaseObject *> BaseObject::childList() const {
    return _childList;
}

bool BaseObject::hasAParent() const {
    return _hasAParent;
}

/**
 * @brief Returns a pointer to the parent object
 */

BaseObject *BaseObject::parent() const {
    return _parent;
}

/**
 * @brief Makes the object a child of parent
 * @param parent
 */

void BaseObject::setParent(BaseObject *parent) {
    if(_hasAParent)
        _parent->removeChild(this);
    _parent = parent;
    if(parent == NULL) {
        _hasAParent = false;
    }
    else {
        _hasAParent = true;
        parent->addChild(this);
        setParent_impl(parent);
    }
}

void BaseObject::setParent_impl(BaseObject *parent) {

}

bool BaseObject::inheritsRandom() const {
    return _inheritsRandom;
}

BaseObject *BaseObject::clone() const
{
    BaseObject *clonedObject = clone_impl();
    clonedObject->_wasCloned = true;
    return clonedObject;
}

bool BaseObject::wasCloned() const
{
    return _wasCloned;
}

void BaseObject::describe() const
{
    describe_impl();
}

string BaseObject::typeName() const
{
    string str;
#ifdef __GNUC__
    str = abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0);
#else
    str = "UNKNOWN OBJECT";
#endif
    return str;
}

string BaseObject::messagePrefix() const
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,80,"[%F %T]",timeinfo);

    string str = buffer;

    str += "[";
    str += typeName();
    str += "]";

    str += " ";
    return str;
}

void BaseObject::printLogMessage(const char *fmt, va_list arguments) const
{
    if(arguments == NULL)
        fprintf(stderr,fmt,NULL);
    else
        vfprintf(stderr,fmt,arguments);
}

void BaseObject::describe_impl() const
{
    logMessage("");
}

/**
 * @brief Logs a message to stderr
 * @param msg
 *
 *
 * If the compiler used is GCC, the message will be prepended with the class
 * name
 */

void BaseObject::logMessage(const string &msg) const
{
    string str = messagePrefix();
    str += msg;
    str += "\n";
    printLogMessage(str.c_str());
}

void BaseObject::logMessage(const char *fmt, ...) const
{
    va_list arguments;
    va_start ( arguments, fmt );
    string str = messagePrefix();
    str += fmt;
    str += "\n";
    printLogMessage(str.c_str(),arguments);
}

#ifndef BASEOBJECT_H
#define BASEOBJECT_H

#include <cstddef>
#include <list>

using namespace std;

class BaseObject
{
public:
    BaseObject(BaseObject *parent=NULL);
    virtual ~BaseObject();

    list<BaseObject *> childList();
    bool hasAParent();
    BaseObject *parent();
    void setParent(BaseObject *parent);
    bool inheritsRandom();

protected:
    bool _inheritsRandom;

private:
    bool _hasAParent;
    BaseObject *_parent;
    list<BaseObject *> _childList;

    void removeChild(BaseObject *child);
    void deleteAllChildren();
    void addChild(BaseObject *child);

    virtual void setParent_impl(BaseObject *parent);
    virtual void addChild_impl(BaseObject *child);
    virtual void removeChild_impl(BaseObject *child);
};

#endif // BASEOBJECT_H

/*
  This file is part of MCPlusPlus.

  MCPlusPlus is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Mcplusplus is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with MCPlusPlus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <MCPlusPlus/baserandom.h>

#include <fstream>
using namespace MCPP;

BaseRandom::BaseRandom(BaseObject *parent) :
    BaseObject(NULL), mt(NULL)
{
    _inheritsRandom = true;
    mt = NULL;

    // we need to call the BaseObject constructor with NULL parent, and then
    // call setParent() here because we want setParent_impl() to be called (but
    // this function doesn't exist until BaseObject and then BaseRandom are
    // fully constructed)
    setParent(parent);
}

BaseRandom::~BaseRandom() {
    if(!hasAParent() || !parent()->inheritsRandom())
        if(mt!=NULL)
            delete mt;
}

/**
 * @brief Sets a new seed for the RNG
 * @param seed
 *
 *
 * This function has no effect if the object has a parent of type BaseRandom.
 * On the parent, this makes all child objects' RNG pointers point to the
 * calling object's RNG.
 */

void BaseRandom::setSeed(unsigned int seed) {
    if(hasAParent() && parent()->inheritsRandom())
        return;
    _currentSeed = seed;
    setRNG(new MCEngine(seed));
}

/**
 * @brief Dumps the RNG internal state to file
 * @param fileName
 */

void BaseRandom::dumpGeneratorState(const char *fileName) const
{
    ofstream file;
    file.open(fileName);
    file << *mt;
    file.close();
}

/**
 * @brief Loads a RNG state from file
 * @param fileName
 */

void BaseRandom::loadGeneratorState(const char *fileName)
{
    logMessage("Loading RNG state from: %s",fileName);
    ifstream file;
    file.open(fileName);
    MCEngine *mt = new MCEngine(0);
    file >> *mt;
    setRNG(mt);
    file.close();
}

/**
 * @brief The RNG internal state
 * @return
 */

string BaseRandom::generatorState() const
{
    stringstream ss;
    ss << *mt;
    return ss.str();
}

/**
 * @brief Set the internal RNG state
 * @param state
 *
 *
 * Use generatorState() to obtain the RNG internal state of a BaseRandom object
 */

void BaseRandom::setGeneratorState(string state)
{
    MCEngine *mt = new MCEngine(0);
    stringstream ss;
    ss << state;
    ss >> *mt;
    setRNG(mt);
}

bool BaseRandom::sanityCheck_impl() const
{
    if(hasAParent())
        return true;

    if(mt != NULL)
        return true;
    else
        return false;
}

/**
 * @brief The seed currently used by the RNG
 * @return
 */

unsigned int BaseRandom::currentSeed() const
{
    return _currentSeed;
}

/**
 * @brief Sets the RNG
 * @param mt
 *
 *
 * The specified RNG is propagated to all child BaseRandoms.
 */

void BaseRandom::setRNG(MCEngine *mt) {
    if(mt == NULL)
        return;
    if(!hasAParent() && this->mt != NULL)
        delete this->mt;
    this->mt = mt;
    setRNG_impl();
    std::list<BaseRandom *>::const_iterator iterator;
    for (iterator = randomChildList.begin();
         iterator != randomChildList.end();
         ++iterator) {
        BaseRandom *rnd = *iterator;
        rnd->_currentSeed = _currentSeed;
        rnd->setRNG(mt);
    }
}

/**
 * @brief Custom further implementation of setRNG(), specified in derived
 * classes.
 *
 * This function will be called by setRNG() after a new RNG has been set to
 * allow derived classes to perform further operations related to the setting
 * of the new RNG.
 *
 * The default implementation does nothing.
 */

void BaseRandom::setRNG_impl() {

}

void BaseRandom::addChild_impl(BaseObject *child) {
    if(!child->inheritsRandom())
        return;

    BaseRandom *rnd = (BaseRandom *)child;
    randomChildList.push_back(rnd);
}

void BaseRandom::removeChild_impl(BaseObject *child) {
    if(!child->inheritsRandom())
        return;

    BaseRandom *rnd = (BaseRandom *)child;
    randomChildList.remove(rnd);
}

BaseObject *BaseRandom::clone_impl() const
{
    return new BaseRandom();
}

void BaseRandom::setParent_impl(BaseObject *parent) {
    if(parent == NULL || !parent->inheritsRandom())
        return;
    BaseRandom *randomParent = (BaseRandom*)parent;
    setRNG(randomParent->mt);
}

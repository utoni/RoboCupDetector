/**
 * Date: 24.05.2016
 * Author: Toni Uhlig <matzeton@googlemail.com>
 */

/* ACHTUNG: Diese Klasse ist _NICHT_ `Threadsafe`, d.h. in Multithreaded Umgebungen muss der Entwickler entsprechende Synchronisationsmaßnahmen ergreifen (siehe RC_BlobDetectorFactory) */

#ifndef RC_CIRCULARBUFFER_H
#define RC_CIRCULARBUFFER_H 1

#include <iostream>
#include <vector>


namespace rc {
template <class T>
class CircularBuffer {

  private:
    T * cBuffer; // Array der Länge "maxElements"
    unsigned int nextIndex; // Feldindex, welcher beim nächsten addElement(...) überschrieben wird
    unsigned int maxElements; // maximale Größe des Puffers
    unsigned int availableElements; // Anzahl "frischer" Einträge
    bool * alreadyProcessedElements; // Bool- Feld, welches bereits durch `getElement` zurückgegebe Elemente mit `true` markiert

  public:
    CircularBuffer(unsigned int maxElements) {
      nextIndex = 0;
      availableElements = 0;
      this->maxElements = maxElements;
      cBuffer = new T[maxElements]; // erzeuge Feld der Größe n (maxElements)
      alreadyProcessedElements = new bool[maxElements];
      for (auto i = maxElements; i > 0; --i) {
        alreadyProcessedElements[i % maxElements] = false;
      }
    }

    ~CircularBuffer(void) {
      delete[] cBuffer;
      delete[] alreadyProcessedElements;
    }

    void addElement(T& elem) {
      /* überschreibe das Element an Position "nextIndex" */
      cBuffer[nextIndex] = elem;
      /* den entsprechende Index in `alreadyProcessedElements` auf `false` setzen */
      alreadyProcessedElements[nextIndex] = false;
      /* nextIndex inkrementieren und dabei auf die Grenze "maxElements" beachten */
      /* % = Modulo */
      auto tmp = nextIndex;
      tmp = ++nextIndex % maxElements;
      nextIndex = tmp;
      if (availableElements < maxElements)
        availableElements++;
    }

    unsigned int getMaxElementCount(void) { return maxElements; }
    unsigned int getNextIndex(void) { return nextIndex; }
    unsigned int getElementCount(void) { return availableElements; }

    bool getElement(T& arg) {
      if (availableElements == 0) return false; // noch keine Elemente im Puffer
      for (auto i = nextIndex; i < nextIndex+maxElements; ++i) {
        auto index = i % maxElements;
        if (alreadyProcessedElements[index] == false) {
          alreadyProcessedElements[index] = true;
          arg = cBuffer[index];
          availableElements--;
          return true;
        }
      }
      return false; // alle Elemente im Puffer wurden bereits mit `getElement()` zurückgegeben
    }

};
}

#endif

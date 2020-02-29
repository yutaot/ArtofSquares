/**
 *
 * Balanced Quad Tree (pa3)
 *
 * This file will be used for grading.
 *
 */

#include "QTree.h"
#include <queue>
#include <vector>

// Return the biggest power of 2 less than or equal to n
int biggestPow2(int n) {
  if( n < 1 ) return 0;
  int v = 1;
  while( v <= n ) v <<= 1;
  return v >> 1;
}

QTree::Node::Node(PNG & im, pair<int,int> ul, int sz, Node *par)
  :upLeft(ul),size(sz),parent(par),nw(NULL),ne(NULL),sw(NULL),se(NULL)
{
  var = varAndAvg(im,ul,size,avg);
}

QTree::~QTree(){
  clear();
}

QTree::QTree(const QTree & other) {
  copy(other);
}


QTree & QTree::operator=(const QTree & rhs){

  // cout<<__func__<<"    "<<__LINE__<<endl;
  if (this != &rhs) {
    // cout<<__func__<<"    "<<__LINE__<<endl;
    clear();
    // cout<<__func__<<"    "<<__LINE__<<endl;
    copy(rhs);
    // cout<<__func__<<"    "<<__LINE__<<endl;
  }
  // cout<<__func__<<"    "<<__LINE__<<endl;
  return *this;
}


QTree::QTree(PNG & imIn, int leafB, RGBAPixel frameC, bool bal)
  : leafBound(leafB), balanced(bal), drawFrame(true), frameColor(frameC)
{

  /* YOUR CODE HERE */
  im = imIn;
  imNoFrame = im;

  int imgWidth = im.width();
  int imgHeight = im.height();
  int shorterSide;
  if (imgWidth > imgHeight) {
    shorterSide = imgHeight;
  } else {
    shorterSide = imgWidth;
  }
  int size = biggestPow2(shorterSide);
  pair<int, int> upperLeft = make_pair(0, 0);

  Node* rootNode = new Node(im, upperLeft, size, NULL);

  this->root = rootNode;
  this->numLeaf = 1;

  Q.push(root);

  while(numLeaf < leafBound) {
    if (!Q.empty()) {
      Node * node = Q.top();
      Q.pop();
      split(node);
    }
  }
}


QTree::QTree(PNG & imIn, int leafB, bool bal)
  : leafBound(leafB), balanced(bal), drawFrame(false)
{

  /* YOUR CODE HERE */
  im = imIn;
  imNoFrame = im;
  int imgWidth = im.width();
  int imgHeight = im.height();
  int shorterSide;
  if (imgWidth > imgHeight) {
    shorterSide = imgHeight;
  } else {
    shorterSide = imgWidth;
  }
  int size = biggestPow2(shorterSide);
  pair<int, int> upperLeft = make_pair(0, 0);

  Node* rootNode = new Node(im, upperLeft, size, NULL);

  this->root = rootNode;
  this->numLeaf = 1;

  Q.push(root);

  while(numLeaf < leafBound) {
    if (!Q.empty()) {
      Node * node = Q.top();
      Q.pop();
      split(node);
    }
  }
}


bool QTree::isLeaf( Node *t ) {

  /* YOUR CODE HERE */
  if (t == NULL) {
    return false;
  }
  return (t->nw == NULL && t->ne == NULL && t->sw == NULL && t->se == NULL);
}

void QTree::split( Node *t ) {

  /* YOUR CODE HERE */

  // FOR BALANCED QTREES-------------------------------------------------
  // A split might cause one or two nbrs of the parent of t to split
  // to maintain balance.  Note that these two nbrs exist (unless they're
  // not in the image region) because the current set of leaves are
  // balanced.
  // if( t is a NW (or NE or SW or SE) child ) then we need to check that
  // the North and West (or North and East or South and West or
  // South and East) nbrs of t->parent have children. If they don't
  // we need to split them.

  //numLeaf >= leafBound ||
  if (!isLeaf(t) || t->size <= 1 || t == NULL) {
    return;
  }

  int half = (t->size)/2;

  Node * northWest = new Node(im, t->upLeft, half, t);
  t->nw = northWest;

  pair<int,int> ulNE = make_pair(t->upLeft.first + half, t->upLeft.second);
  Node * northEast = new Node(im, ulNE, half, t);
  t->ne = northEast;

  pair<int,int> ulSW = make_pair(t->upLeft.first, t->upLeft.second + half);
  Node * southWest = new Node(im, ulSW, half, t);
  t->sw = southWest;

  pair<int,int> ulSE = make_pair(t->upLeft.first + half, t->upLeft.second + half);
  Node * southEast = new Node(im, ulSE, half, t);
  t->se = southEast;

  Q.push(northWest);
  Q.push(northEast);
  Q.push(southWest);
  Q.push(southEast);
  numLeaf += 3;

  if(balanced) {
    balanceSplit(t);
  }
}

void QTree::balanceSplit(Node * node) {
  if (node->parent == NULL) {
    return;
  }
  Node* parentNode = node->parent;
  if (parentNode->nw == node) {
    Node* northNbr = NNbr(parentNode);
    Node* westNbr = WNbr(parentNode);
    split(northNbr);
    split(westNbr);

  } else if (parentNode->ne == node) {
    Node* northNbr = NNbr(parentNode);
    Node* eastNbr = ENbr(parentNode);
    split(northNbr);
    split(eastNbr);

  } else if (parentNode->sw == node) {
    Node* southNbr = SNbr(parentNode);
    Node* westNbr = WNbr(parentNode);
    split(southNbr);
    split(westNbr);

  } else if (parentNode->se == node) {
    Node* southNbr = SNbr(parentNode);
    Node* eastNbr = ENbr(parentNode);
    split(southNbr);
    split(eastNbr);
  }
}

void QTree::findAndRemoveFromQ(Node * node) {
  queue<Node*> queue;
  for (int i = 0; i < Q.size(); i++) {
    Node* p = Q.top();
    Q.pop();
    if (p == node) {
      for (int j = 0; j < queue.size(); j++) {
        Node * q = queue.front();
        queue.pop();
        Q.push(q);
      }
      return;
    }
    queue.push(p);
  }
  for (int k = 0; k < queue.size(); k++) {
    Node * s = queue.front();
    queue.pop();
    Q.push(s);
  }
  return;
}


/* NNbr(t)
 * return the same-sized quad tree node that is north of Node t.
 * return NULL if this node is not in the QTree.
 */
QTree::Node * QTree::NNbr(Node *t) {

  /* YOUR CODE HERE */

  int desiredYPos = t->upLeft.second - t->size;
  pair<int, int> find = make_pair(t->upLeft.first, desiredYPos);
  Node * toFind = findNode(t, find, root);
  return toFind;
}

/* SNbr(t)
 * return the same-sized quad tree node that is south of Node t.
 * return NULL if this node is not in the QTree.
 */
QTree::Node * QTree::SNbr(Node *t) {

  /* YOUR CODE HERE */
  int desiredYPos = t->upLeft.second + t->size;
  pair<int, int> find = make_pair(t->upLeft.first, desiredYPos);
  Node * toFind = findNode(t, find, root);
  return toFind;
}

/* ENbr(t)
 * return the same-sized quad tree node that is east of Node t.
 * return NULL if this node is not in the QTree.
 */
QTree::Node * QTree::ENbr(Node *t) {

  /* YOUR CODE HERE */
  int desiredXPos = t->upLeft.first + t->size;
  pair<int, int> find = make_pair(desiredXPos, t->upLeft.second);
  Node * toFind = findNode(t, find, root);
  return toFind;
}

/* WNbr(t)
 * return the same-sized quad tree node that is west of Node t.
 * return NULL if this node is not in the QTree.
 */
QTree::Node * QTree::WNbr(Node *t) {

  /* YOUR CODE HERE */
  int desiredXPos = t->upLeft.first - t->size;
  pair<int, int> find = make_pair(desiredXPos, t->upLeft.second);
  Node * toFind = findNode(t, find, root);
  return toFind;
}

QTree::Node * QTree::findNode(Node* t, pair<int, int> find, Node* node) {
  if (node != NULL) {
    if (node->upLeft.first == find.first && node->upLeft.second == find.second && node->size == t->size) {
      return node;
    } else {
      Node * p = findNode(t, find, node->nw);
      if (p == NULL) {
        p = findNode(t, find, node->ne);
      }
      if (p == NULL) {
        p = findNode(t, find, node->sw);
      }
      if (p == NULL) {
        p = findNode(t, find, node->se);
      }
      return p;
    }
  } else {
    return NULL;
  }
}

bool QTree::write(string const & fileName){

  /* YOUR CODE HERE */
  writeHelper(im, imNoFrame, root);

  // include the following line to write the image to file.
  return(im.writeToFile(fileName));
}

void QTree::writeHelper(PNG & im, PNG & imNoFrame, Node* node) {
  if (node == NULL) {
    return;
  }
  if (isLeaf(node)) {

    pair<int,int> ul = node->upLeft;
    int size = node->size;

    for (int y = ul.second; y < ul.second + size; y++) {
      for (int x = ul.first; x < ul.first + size; x++) {
        RGBAPixel * p = im.getPixel(x,y);
        *p = node->avg;
        RGBAPixel * q = imNoFrame.getPixel(x,y);
        *q = node->avg;
      }
    }
    if (drawFrame) {
      for (int y = ul.second; y < ul.second + size; y++) {
        RGBAPixel * verticalLeft = im.getPixel(ul.first, y);
        *verticalLeft = frameColor;
        RGBAPixel * verticalRight = im.getPixel(ul.first + size - 1, y);
        *verticalRight = frameColor;
      }
      for (int x = ul.first; x < ul.first + size; x++) {
        RGBAPixel * horLeft = im.getPixel(x, ul.second);
        *horLeft = frameColor;
        RGBAPixel * horRight = im.getPixel(x, ul.second + size - 1);
        *horRight = frameColor;
      }
    }
  }
  writeHelper(im, imNoFrame, node->nw);
  writeHelper(im, imNoFrame, node->ne);
  writeHelper(im, imNoFrame, node->sw);
  writeHelper(im, imNoFrame, node->se);
}

void QTree::clear() {

  /* YOUR CODE HERE */
  clearHelper(root);
}

void QTree::clearHelper(Node *& node) {
  if (node == NULL) {
    return;
  }
  clearHelper(node->nw);
  clearHelper(node->ne);
  clearHelper(node->sw);
  clearHelper(node->se);
  delete(node);
  node = NULL;
}

void QTree::copy(const QTree & orig) {

  /* YOUR CODE HERE */
  this->im = orig.im;
  this->numLeaf = orig.numLeaf;
  this->leafBound = orig.leafBound;
  this->balanced = orig.balanced;
  this->drawFrame = orig.drawFrame;
  // if (drawFrame) {
    this->frameColor = orig.frameColor;
    this->imNoFrame = orig.imNoFrame;
  // }

  Node* rootNode = orig.root;
  this->root = new Node(imNoFrame, rootNode->upLeft, rootNode->size, NULL);

  copyHelper(rootNode, this->root);
}

void QTree::copyHelper(Node* node, Node*& thisTreeNode) {
  if (node == NULL) {
    return;
  }

  if (node->nw != NULL) {
    Node* thisNW = node->nw;
    Node* thisNE = node->ne;
    Node* thisSW = node->sw;
    Node* thisSE = node->se;

    thisTreeNode->nw = new Node(imNoFrame, thisNW->upLeft, thisNW->size, thisTreeNode);
    thisTreeNode->ne = new Node(imNoFrame, thisNE->upLeft, thisNE->size, thisTreeNode);
    thisTreeNode->sw = new Node(imNoFrame, thisSW->upLeft, thisSW->size, thisTreeNode);
    thisTreeNode->se = new Node(imNoFrame, thisSE->upLeft, thisSE->size, thisTreeNode);

    copyHelper(node->nw, thisTreeNode->nw);
    copyHelper(node->ne, thisTreeNode->ne);
    copyHelper(node->sw, thisTreeNode->sw);
    copyHelper(node->se, thisTreeNode->se);
  }
}

// [[Rcpp::plugins(openmp)]]
// [[Rcpp::plugins(cpp11)]]
#include <vector>
#include <memory>
#include <math.h>

using namespace std;

template<class VIDX, class D>
class PairingHeap {
private:
  class PairNode {
  public:
    D distance;
    VIDX index;
    typedef PairNode* NodePointer;
    NodePointer leftChild;
    NodePointer nextSibling;
    NodePointer prev;
    PairNode(VIDX index, D distance) : distance{distance}, index{index} {
    	leftChild = nextSibling = prev = NULL;
    }
    void reclaimMemory() {
    	leftChild = nextSibling = prev = NULL;
    }
  };

	typedef PairNode* NodePointer;

	NodePointer root;
	VIDX MaxSize = 0;
	vector< NodePointer > PointerArray;
	vector< bool > ContentsArray;

	void compareAndLink(NodePointer &first, NodePointer second) {
		if (second == NULL) return;
		if (second->distance < first->distance) {
			second->prev = first->prev;
			first->prev = second;
			first->nextSibling = second->leftChild;
			if (first->nextSibling != NULL) first->nextSibling->prev = first;
			second->leftChild = first;
			first = second;
		} else {
			second->prev = first;
			first->nextSibling = second->nextSibling;
			if (first->nextSibling != NULL) first->nextSibling->prev = first;
			second->nextSibling = first->leftChild;
			if (second->nextSibling != NULL) second->nextSibling->prev = second;
			first->leftChild = second;
		}
	}

	NodePointer combineSiblings(NodePointer firstSibling) {
		if (firstSibling->nextSibling == NULL) {
			return firstSibling;
		}
		static vector< PairingHeap<VIDX, D>::NodePointer > treeArray(5);
		int numSiblings = 0;
		for (; firstSibling != NULL; numSiblings++) {
			if (numSiblings == treeArray.size()) treeArray.resize(numSiblings * 2);
			treeArray[numSiblings] = firstSibling;
			firstSibling->prev->nextSibling = NULL;
			firstSibling = firstSibling->nextSibling;
		}
		if (numSiblings == treeArray.size()) treeArray.resize(numSiblings + 1);
		treeArray[numSiblings] = NULL;
		int i = 0;
		for (; i + 1 < numSiblings; i += 2) compareAndLink(treeArray[i], treeArray[i + 1]);
		int j = i - 2;
		if (j == numSiblings - 3) compareAndLink (treeArray[j], treeArray[j + 2]);
		for (; j >= 2; j -= 2) compareAndLink(treeArray[j - 2], treeArray[j] );
		return treeArray[0];
	}

	NodePointer Insert(VIDX &n, D &x) {
		NodePointer newNode = new PairNode(n, x);
		if (root == NULL) root = newNode;
		else compareAndLink(root, newNode);
		PointerArray[n] = newNode;
		ContentsArray[n] = true;
		return newNode;
	}

  void makeEmpty() {
  	root = NULL;
  }

public:
	PairingHeap(VIDX N) : root(NULL), MaxSize{N},
												PointerArray(vector< NodePointer >(N, nullptr)),
												ContentsArray(vector< bool >(N, false)) {
	}
	~PairingHeap() {
		for (VIDX i = 0; i != PointerArray.size(); i++) {
			if (PointerArray[i] != NULL) delete PointerArray[i];
		}
	}

	VIDX pop() {
		NodePointer oldRoot = root;
		if (root->leftChild == NULL) root = NULL;
		else root = combineSiblings(root->leftChild);
		VIDX ret = oldRoot -> index;
		ContentsArray[ret] = false;
		return ret;
	}

	bool isEmpty() const {
		return root == NULL;
	}

	bool contains(const VIDX& i) const {
		return ContentsArray[i];
	}

	void insert(VIDX& n, D &x) {
		Insert(n, x);
	}


	void batchInsert(const VIDX& n, const VIDX& start) {
		for (VIDX i = 0; i != n; i++) {
			D key = (start == i) ? -1 : INFINITY;
			Insert(i, key);
		}
	};

  bool decreaseIf(const VIDX& i, const D &newVal) {
  	NodePointer p = PointerArray[i];
  	if (p->distance < newVal) return false;
  	p->distance = newVal;
  	if (p != root) {
  		if (p->nextSibling != NULL)
  			p->nextSibling->prev = p->prev;
  		if (p->prev->leftChild == p)
  			p->prev->leftChild = p->nextSibling;
  		else
  			p->prev->nextSibling = p->nextSibling;
  		p->nextSibling = NULL;
  		compareAndLink(root, p);
  	}
  	return true;
  }

  D keyOf(const VIDX& i) const {
  	return PointerArray[i] -> distance;
  };

  D topKey() const {
  	if (root == NULL) return INFINITY;
  	return root -> distance;
  }
};

#include <fstream>
#include <iostream>


template<class type, int order>
class BTree
{
	/*private treeNode subclass*/
	/*each node contains a maximum of order children*/
	/*each node contains a maximum of order-1 data entries*/
	/*number of data entries is always number of children-1*/
	/*every node but the root and leaves has at least order/2 children*/
	/*every non-leaf has at least 2 children*/
	struct treeNode
	{
		int degree;     //number of data entries node currently has
		int child_index;  //this child's index in the parent's children array
		treeNode* parent;
		type data[order]; //additional slot for splitting
		treeNode* children[order + 1]; //additional slot for splitting
		treeNode() : treeNode(0, nullptr) {}
		treeNode(int p_child_index, treeNode* p_parent) :
			degree(),
			child_index(p_child_index),
			parent(p_parent),
			data(),
			children() {}
		/*destructor to ensure proper deletion*/
		~treeNode()
		{
			for (int i = 0; i <= order; ++i)
				delete children[i];
		}
	};
public:
	class iterator
	{
		friend class BTree;
	private:
		treeNode* node;
		int pos;
		iterator() : iterator(nullptr, 0) {}
		iterator(treeNode* p_node, int p_pos) : node(p_node), pos(p_pos) {}
	public:
		iterator(const iterator& rh) : node(rh.node), pos(rh.pos) {}
		iterator& operator=(const iterator& rh)
		{
			node = rh.node;
			pos = rh.pos;
			return *this;
		}
		bool operator==(const iterator& rh)
		{
			return node == rh.node && pos == rh.pos;
		}
		bool operator!=(const iterator& rh)
		{
			return !(*this == rh);
		}
		iterator& operator--()
		{
			*this = predecessorSearch(*this);
			return *this;
		}
		iterator& operator++()
		{
			*this = successorSearch(*this);
			return *this;
		}
		const type& operator*() const
		{
			return node->data[pos];
		}
	};
private:
	/*private methods */

	/*inserting a value into a specific node at a position*/
	/*the value is guaranteed to belong in this node at that position*/
	void insertInto(iterator spot, type val)
	{
		auto ins_node = spot.node;
		int i = spot.pos;
		/*useless fix to get rid of warnings*/
		if (i < 0)
			i = 0;
		/*if the value exists already do nothing.
		degree==0 catches the edge case of inserting 0 into an empty tree*/
		if (ins_node->data[i] != val || i==ins_node->degree|| ins_node->degree == 0)
		{
			/*shifting elements to make room for new entry*/
			for (int j = ins_node->degree; j > i; --j)
				ins_node->data[j] = ins_node->data[j - 1];

			ins_node->data[i] = val;
			++ins_node->degree;
			++size;
			/*if node reached max capacity it splits*/
			if (ins_node->degree == order)
				split(ins_node);
		}
	}
	/*splitting a maximum capacity node into 2 new nodes*/
	void split(treeNode* node)
	{
		/*unless I do this it won't stop giving me buffer overrun warnings
		despite the fact that degree is guaranteed to be equal to order at this point*/
		if (node->degree > order)
			node->degree = order;

		int median = (node->degree - 1) / 2; //index of median entry

		/*special case for splitting the root*/
		if (node == root)
		{
			node->parent = new treeNode();
			root = node->parent;
		}
		/*creation of the 2 new nodes*/
		auto left = new treeNode(node->child_index, node->parent);
		auto right = new treeNode(node->child_index + 1, node->parent);

		/*copying all entries < median and their */
		/*corresponding children into the left node*/
		for (int i = 0; i < median; ++i)
		{
			left->data[i] = node->data[i];
			left->children[i] = node->children[i];
			if (node->children[i] != nullptr)
			{
				node->children[i]->parent = left;
				node->children[i]->child_index = i;
			}
			node->children[i] = nullptr;
			++left->degree;
		}
		left->children[median] = node->children[median];
		if (node->children[median] != nullptr)
		{
			node->children[median]->parent = left;
			node->children[median]->child_index = median;
		}
		node->children[median] = nullptr;

		/*copying all entries > median and their */
		/*corresponding children into the right node*/
		for (int j = median + 1; j < node->degree; ++j)
		{
			right->data[j - median - 1] = node->data[j];
			right->children[j - median - 1] = node->children[j];
			if (node->children[j] != nullptr)
			{
				node->children[j]->parent = right;
				node->children[j]->child_index = j - median - 1;
			}
			node->children[j] = nullptr;
			++right->degree;
		}
		right->children[node->degree - median - 1] = node->children[node->degree];
		if (node->children[node->degree] != nullptr)
		{
			node->children[node->degree]->parent = right;
			node->children[node->degree]->child_index = node->degree - median - 1;
		}
		node->children[node->degree] = nullptr;
		/*children were set to null in the original to avoid deletion*/


		/*shifting children in the parent node to make space for the right node*/
		for (int i = node->parent->degree + 1; i > node->child_index + 1; --i)
		{
			node->parent->children[i] = node->parent->children[i - 1];
			if (node->parent->children[i] != nullptr)
				++node->parent->children[i]->child_index;
		}

		/*adding the nodes, the parent effectively gains 1 child*/
		node->parent->children[node->child_index] = left;
		node->parent->children[node->child_index + 1] = right;

		/*inserting the median value into the parent recursively*/
		--size;
		auto spot = iterator(node->parent, node->child_index);
		insertInto(spot, node->data[median]);

		/*deleting the now obsolete node*/
		delete node;
	}

	/*searching for the spot(node and position) of a specific value*/
	iterator nodeSearch(type val) const
	{
		treeNode* crt_node = root;
		/*treating the special case of searching an empty tree*/
		if (size > 0)
			/*iterating through nodes*/
			while (crt_node != nullptr)
			{
				int i = 0;
				/*iterating through all entries < the desired one*/
				while (i < crt_node->degree && crt_node->data[i] < val)
					++i;
				/*if the desired value was found,
				 or the corresponding child does not exist,
				 return this spot*/
				if (crt_node->data[i] == val || crt_node->children[i] == nullptr)
					return iterator(crt_node, i);
				/*if not, move to corresponding child*/
				else
					crt_node = crt_node->children[i];
			}
		return iterator(crt_node, 0);
	}

	void rebalance(treeNode* node)
	{
		/*rebalancing the tree after a deletion*/

		/*if the current node is the root, simply return*/
		if (node->parent == nullptr)
			return;

		auto parent = node->parent;

		/*attempting right rotation*/
		if (node->child_index > 0)
		{
			auto left_sibling = parent->children[node->child_index - 1];

			/*checking if the left sibling has enough elements
			to support a rotation*/
			if (left_sibling->degree > (order - 1) / 2)
			{

				/*shifting the rightmost children to the right*/
				node->children[node->degree + 1] = node->children[node->degree];
				if (node->children[0] != nullptr)
					++node->children[node->degree + 1]->child_index;


				/*shifting all other children and elements to the right*/
				/*to make space for the new element and child*/
				for (int i = node->degree; i > 0; --i)
				{
					node->data[i] = node->data[i - 1];
					node->children[i] = node->children[i - 1];
					if (node->children[0] != nullptr)
						++node->children[i]->child_index;
				}

				int pos = node->child_index - 1;

				/*performing the right rotation*/
				node->data[0] = parent->data[pos];
				parent->data[pos] = left_sibling->data[left_sibling->degree - 1];
				left_sibling->data[left_sibling->degree - 1] = type();
				node->children[0] = left_sibling->children[left_sibling->degree];

				/*fixing rotated child*/
				if (left_sibling->children[0] != nullptr)
				{
					left_sibling->children[left_sibling->degree]->parent = node;
					left_sibling->children[left_sibling->degree]->child_index = 0;
					left_sibling->children[left_sibling->degree] = nullptr;
				}

				/*updating degrees*/
				--left_sibling->degree;
				++node->degree;

				return;

			}
		}
		/*attempting a left rotation*/
		if (node->child_index < parent->degree)
		{
			/*storing right sibling*/
			auto right_sibling = parent->children[node->child_index + 1];

			/*checking if the right sibling has enough elements
			 to support a rotation*/
			if (right_sibling->degree > (order - 1) / 2)
			{
				int pos = node->child_index;

				/*performing the left rotation*/
				node->data[node->degree] = parent->data[pos];
				parent->data[pos] = right_sibling->data[0];
				node->children[node->degree + 1] = right_sibling->children[0];

				/*fixing rotated child*/
				if (right_sibling->children[0] != nullptr)
				{
					right_sibling->children[0]->parent = node;
					right_sibling->children[0]->child_index = node->degree + 1;
				}

				/*shifting all elements and children in the right sibling
				 to the left to fix inner structure*/
				for (int i = 0; i < right_sibling->degree - 1; ++i)
				{
					right_sibling->data[i] = right_sibling->data[i + 1];
					right_sibling->children[i] = right_sibling->children[i + 1];
					if (right_sibling->children[0] != nullptr)
						--right_sibling->children[i]->child_index;
				}
				right_sibling->data[right_sibling->degree - 1] = type();

				/*fixing rightmost child not covered by loop*/
				right_sibling->children[right_sibling->degree - 1] = right_sibling->children[right_sibling->degree];
				right_sibling->children[right_sibling->degree] = nullptr;
				if (right_sibling->children[0] != nullptr)
					--right_sibling->children[right_sibling->degree - 1]->child_index;


				/*updating degrees*/
				--right_sibling->degree;
				++node->degree;
				return;
			}
		}

		/*performing a merge*/
		treeNode* left, * right;
		if (node->child_index > 0)
		{
			/*left sibling exists, merging with it*/
			left = parent->children[node->child_index - 1];
			right = node;
		}
		else
		{
			/*right sibling exists, merging with it*/
			left = node;
			right = parent->children[node->child_index + 1];
		}

		int pos = left->child_index;
		/*useless check to get rid of false warning*/
		if (pos < 0)
			pos = 0;

		/*merging with the parent element*/
		left->data[left->degree] = parent->data[pos];
		++left->degree;


		/*merging with the sibling*/
		for (int i = 0; i < right->degree; ++i)
		{
			left->data[left->degree] = right->data[i];
			left->children[left->degree] = right->children[i];
			right->children[i] = nullptr;
			if (left->children[0] != nullptr)
			{
				left->children[left->degree]->parent = left;
				left->children[left->degree]->child_index = left->degree;
			}
			++left->degree;
		}

		/*another useless check to get rid of a false warning*/
		if (left->degree > order)
			left->degree = order;

		/*fixing rightmost child not covered by loop*/
		left->children[left->degree] = right->children[right->degree];
		right->children[right->degree] = nullptr;
		if (left->children[0] != nullptr)
		{
			left->children[left->degree]->parent = left;
			left->children[left->degree]->child_index = left->degree;
		}

		/*deleting the now redundant sibling*/
		delete right;


		/*fixing the parent by left shifting
		 elements and children to cover gap left
		 by losing an element and a child*/
		for (int i = pos; i < parent->degree - 1; ++i)
		{
			parent->data[i] = parent->data[i + 1];
			parent->children[i + 1] = parent->children[i + 2];
			if (parent->children[i + 1] != nullptr)
				--parent->children[i + 1]->child_index;
		}

		/*yet another useless check to make my IDE happy*/
		if (parent->degree < 1)
			parent->degree = 1;

		/*finishing touches on the parent node*/
		parent->data[parent->degree - 1] = type();
		parent->children[parent->degree] = nullptr;
		--parent->degree;

		/*if the parent node is the root and became empty
		 delete it and make it's only child the new root*/
		if (parent->parent == nullptr && parent->degree == 0)
		{
			/*treating the edge case of an empty tree*/
			if (size == 0)
				return;

			/*changing the root*/
			parent->children[0]->parent = nullptr;
			root = parent->children[0];
			parent->children[0] = nullptr;
			delete parent;
			return;
		}

		/*if the parent became too small after merging,
		 recursively balance the tree*/
		if (parent->degree < (order - 1) / 2)
			rebalance(parent);

	}


	void removeFrom(iterator it)
	{
		/*removing an element from a certain position*/

		/*if element to be deleted is not in a leaf node
		 swap it with it's predecessor or successor*/
		if (it.node->children[0] != nullptr)
		{
			auto jt = predecessorSearch(it);
			if (jt == irend)
				jt = successorSearch(it);
			std::swap(it.node->data[it.pos], jt.node->data[jt.pos]);
			it = jt;
		}
		/*left shift all greater elements to close the gap*/
		for (int i = it.pos; i < it.node->degree - 1; ++i)
			it.node->data[i] = it.node->data[i + 1];

		it.node->data[it.node->degree - 1] = type();
		--it.node->degree;

		/*if the node became too small, balance the tree*/
		if (it.node->degree < (order - 1) / 2)
			rebalance(it.node);
	}

	static iterator successorSearch(iterator it)
	{
		/*determining the successor iterator of a given iterator*/

		/*if current node is not a leaf node*/
		if (it.node->children[it.pos + 1] != nullptr)
		{
			/*travel down in the right subtree*/
			it = iterator(it.node->children[it.pos + 1], 0);

			/*while the current node is not a leaf
			 travel down in the leftmost subtree*/
			while (it.node->children[0] != nullptr)
				it = iterator(it.node->children[0], 0);
			return it;
		}

		/*if current node is a leaf node but the element
		 is not the rightmost one, just increment the position*/
		if (it.pos != it.node->degree - 1)
			return iterator(it.node, it.pos + 1);

		/*if current node is a leaf node and the element
		 is the rightmost one, travel up until the current node
		 no longer is the rightmost child of it's parent*/
		while (it.node->parent != nullptr && it.node->child_index == it.node->parent->degree)
			it = iterator(it.node->parent, 0);

		/*if the root has not been reached, return the corresponding
		 parent element*/
		if (it.node->parent != nullptr)
			return iterator(it.node->parent, it.node->child_index);

		/*if the root has been reached, then the starting element was already
		 the maximum, hence it has no successor*/
		return iend;
	}
	static iterator predecessorSearch(iterator it)
	{
		/*determining the corresponding predecessor iterator
		 for a given iterator*/

		 /*if the current node is not a leaf node*/
		if (it.node->children[it.pos] != nullptr)
		{
			/*travel down the left subtree*/
			it = iterator(it.node->children[it.pos], it.node->children[it.pos]->degree - 1);

			/*while the current node is not a leaf
			 travel down the right subtree*/
			while (it.node->children[it.node->degree] != nullptr)
				it = iterator(it.node->children[it.node->degree], it.node->children[it.node->degree]->degree - 1);
			return it;
		}

		/*if the current node is a leaf but the element
		 is not the leftmost one,just decrement the position*/
		if (it.pos != 0)
			return iterator(it.node, it.pos - 1);


		/*if the current node is a leaf and the element
		 is the leftmost one,travel up until the current node
		 is no longer the leftmost child of it's parent*/
		while (it.node->parent != nullptr && it.node->child_index == 0)
			it = iterator(it.node->parent, 0);

		/*if the root has not been reached, return the corresponding
		 parent element*/
		if (it.node->parent != nullptr)
			return iterator(it.node->parent, it.node->child_index - 1);

		/*if the root has been reached, then the starting element was already
		 the minimum, hence it has no predecessor*/
		return irend;
	}

	/*private data*/
	treeNode* root;
	size_t size;
	static const iterator iend, irend;  //static iterators to tree bounds


public:
	BTree() : root(new treeNode()), size() {}
	~BTree()
	{
		delete root;
	}
	/*search method*/
	bool search(type val) const
	{
		auto candidate = nodeSearch(val);

		/*if tree is not empty and the element at the returned
		 position is indeed the searched value, return true*/
		if (size > 0 && *candidate == val && candidate.pos < candidate.node->degree)
			return true;
		return false;
	}

	/*greatest element smaller than or equal to a given value*/
	type predecessor(type val) const
	{
		iterator it = nodeSearch(val);

		/*if the value is found in the tree, return it*/
		if (*it == val && it.pos < it.node->degree)
			return *it;

		/*if the returned candidate is not the value, then return
		 the predecessor of the candidate*/
		--it;

		/*if the iterator is not the lower bound, dereference it*/
		if (it != irend)
			return *it;

		/*otherwise return the minimum*/
		return *begin();

	}

	/*smallest element greater than or equal to a given value*/
	type successor(type val) const
	{
		iterator it = nodeSearch(val);

		/*if the value is not found in the tree or the candidate
		 is after the rightmost element*/ 
		if (*it != val || it.pos == it.node->degree)
		{
			/*if it's not after the rightmost element of the node
			 return the candidate*/
			if (it.pos < it.node->degree)
				return *it;
			/*otherwise return the successor of the rightmost element*/
			--it.pos;
			++it;

		}
		/*if the iterator is not the upper bound, dereference it*/
		if (it != iend)
			return *it;
		/*otherwise return the maximum*/
		return *back();
	}

	/*prints all elements in range [l_bound,u_bound] found in the tree*/
	void printRange(type l_bound, type u_bound, std::ostream& out) const
	{
		iterator it = nodeSearch(l_bound);

		/*if the lower bound is not in the tree*/
		if (*it != l_bound)
		{
			/*if the candidate element is after the rightmost child
			 then the first element of the range will be the successor
			 of the rightmost element*/
			if (it.pos == it.node->degree)
			{
				--it.pos;
				++it;
			}
		}
		/*iterate through the elements and print them until reaching
		 an element greater than u_bound or reaching iend*/
		while (it != iend && *it <= u_bound)
		{
			out << *it << " ";
			++it;
		}
		out << "\n";
	}
	/*returns an iterator to the minimum */
	iterator begin() const
	{
		/*starting from the root, travel down the leftmost subtree*/
		iterator it(root, 0);
		while (it.node->children[0] != nullptr)
			it = iterator(it.node->children[0], 0);
		return it;
	}
	/*returns an iterator to the maximum*/
	iterator back() const
	{
		/*starting from the root, travel down the rightmost subtree*/
		iterator it(root, root->degree - 1);
		while (it.node->children[it.node->degree] != nullptr)
			it = iterator(it.node->children[it.node->degree], it.node->children[it.node->degree]->degree - 1);
		return it;
	}
	/*returns the end iterator*/
	static iterator end()
	{
		return iend;
	}
	/*returns the reverse end iterator*/
	static iterator rend()
	{
		return irend;
	}
	/*inserts a given value into the tree*/
	void insert(type val)
	{
		auto ins_spot = nodeSearch(val);
		insertInto(ins_spot, val);
	}
	/*removes a value from the tree if it exists*/
	void remove(type val)
	{
		auto del_spot = nodeSearch(val);
		if (*del_spot != val || del_spot.pos == del_spot.node->degree)
			return;
		--size;
		removeFrom(del_spot);
	}
};

/*initialization of end and reverse end iterators*/
template<class type, int order>
const typename BTree<type, order>::iterator BTree<type, order>::iend(nullptr, 1);

template<class type, int order>
const typename BTree<type, order>::iterator BTree<type, order>::irend(nullptr, -1);

int main()
{
	// std::ifstream in("abce.in");
	// std::ofstream out("abce.out");
	/*int nr_op, cmd, param1, param2;

	in >> nr_op;
	for (int i = 0; i < nr_op; ++i)
	{
		in >> cmd;
		switch (cmd)
		{
		case 1:
		{
			in >> param1;
			tree.insert(param1);
			break;
		}
		case 2:
		{
			in >> param1;
			tree.remove(param1);
			break;
		}
		case 3:
		{
			in >> param1;
			out << tree.search(param1) << "\n";
			break;
		}
		case 4:
		{
			in >> param1;
			out << tree.predecessor(param1) << "\n";
			break;
		}
		case 5:
		{
			in >> param1;
			out << tree.successor(param1) << "\n";
			break;
		}
		case 6:
		{
			in >> param1 >> param2;
			tree.printRange(param1, param2, out);
			break;
		}

		}
		
	}
	in.close();
	out.close();*/

	BTree<int*, 5> tree;
	int x=20, y=15, z=33, w=-15;
	tree.insert(&w);
	tree.insert(&x);
	tree.insert(&y);
	tree.insert(&z);

	for (const auto i : tree)
		std::cout << *i << " ";		
}


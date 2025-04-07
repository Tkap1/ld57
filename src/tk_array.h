
#define foreach_ptr__(a, index_name, element_name, array) if(0) finished##a: ; else for(auto element_name = &(array).data[0];;) if(1) goto body##a; else while(1) if(1) goto finished##a; else body##a: for(int index_name = 0; index_name < (array).count && (bool)(element_name = &(array)[index_name]); index_name++)
#define foreach_ptr_(a, index_name, element_name, array) foreach_ptr__(a, index_name, element_name, array)
#define foreach_ptr(index_name, element_name, array) foreach_ptr_(__LINE__, index_name, element_name, array)

#define foreach_val__(a, index_name, element_name, array) if(0) finished##a: ; else for(auto element_name = (array).data[0];;) if(1) goto body##a; else while(1) if(1) goto finished##a; else body##a: for(int index_name = 0; index_name < (array).count && (void*)&(element_name = (array)[index_name]); index_name++)
#define foreach_val_(a, index_name, element_name, array) foreach_val__(a, index_name, element_name, array)
#define foreach_val(index_name, element_name, array) foreach_val_(__LINE__, index_name, element_name, array)

template <typename t, int n>
struct s_list
{
	int count = 0;
	t data[n];

	t& operator[](int index);
	t pop_last();
	t* add(t new_element);
	void remove_and_swap(int index);
	t get_last();
};

template <typename t, int n>
t& s_list<t, n>::operator[](int index)
{
	assert(index >= 0);
	assert(index < this->count);
	return this->data[index];
}

template <typename t, int n>
t s_list<t, n>::pop_last()
{
	assert(this->count > 0);
	this->count -= 1;
	t result = this->data[this->count];
	return result;
}

template <typename t, int n>
t* s_list<t, n>::add(t new_element)
{
	assert(this->count < n);
	t* result = &this->data[this->count];
	this->data[this->count] = new_element;
	this->count += 1;
	return result;
}

template <typename t, int n>
void s_list<t, n>::remove_and_swap(int index)
{
	assert(index < this->count);
	this->count -= 1;
	this->data[index] = this->data[this->count];
}

template <typename t, int n>
t s_list<t, n>::get_last()
{
	assert(this->count > 0)
	t result = data[this->count - 1];
	return result;
}

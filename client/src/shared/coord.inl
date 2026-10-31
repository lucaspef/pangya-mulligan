template <class T>
inline _Coordinates<T>::_Coordinates()
{
}

template <class T>
inline _Rectangle<T>::_Rectangle()
{
}

template <class T>
inline _Rectangle<T>::_Rectangle(const _Rectangle<T>& rhs)
{
	left = rhs.left;
	top = rhs.top;
	right = rhs.right;
	bottom = rhs.bottom;
}

template <class T>
inline _Rectangle<T>& _Rectangle<T>::operator=(const _Rectangle<T>& rhs)
{
	left = rhs.left;
	top = rhs.top;
	right = rhs.right;
	bottom = rhs.bottom;
	return *this;
}

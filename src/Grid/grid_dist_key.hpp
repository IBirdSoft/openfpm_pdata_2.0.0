#ifndef GRID_DIST_KEY_DX_HPP
#define GRID_DIST_KEY_DX_HPP

/*! \brief Grid key for a distributed grid
 *
 * It contain from which local sub-domain grid come from, and the local grid_key_dx
 *
 */

template<unsigned int dim>
class grid_dist_key_dx
{
	//! grid list counter

	size_t g_c;

	//! Local grid iterator

	grid_key_dx<dim> key;

public:

	/*! \brief Set the local grid
	 *
	 * \param sub local grid
	 *
	 */
	inline void setSub(size_t sub)
	{
		g_c = sub;
	}

	/*! \brief Get the local grid
	 *
	 * \return the id of the local grid
	 *
	 */
	inline size_t getSub() const
	{
		return g_c;
	}

	/*! \brief Get the key
	 *
	 * \return the local key
	 *
	 */
	inline grid_key_dx<dim> getKey() const
	{
		return key;
	}


	/*! \brief Get the reference key
	 *
	 * \return the local key
	 *
	 */
	inline grid_key_dx<dim> & getKeyRef()
	{
		return key;
	}

	/* \brief Check if two key are the same
	 *
	 * \param key_t key to check
	 *
	 * \return true if the two key are equal
	 *
	 */

	inline bool operator==(const grid_dist_key_dx<dim> & key_t)
	{
		if (g_c != key_t.g_c)
			return false;

		// Check the two key index by index

		return getKey() == key_t.getKey();
	}

	/*! \brief Create a new key moving the old one
	 *
	 * \param i dimension id
	 * \param s number of steps
	 *
	 * \return new key
	 *
	 */
	inline grid_dist_key_dx<dim> move(size_t i,size_t s) const
	{
		grid_key_dx<dim> key = getKey();
		key.set_d(i,key.get(i) + s);
		return grid_dist_key_dx<dim>(getSub(),key);
	}

	/*! \brief Create a new key moving the old one
	 *
	 * \param c where to move for each component
	 *
	 * \return new key
	 *
	 */
	inline grid_dist_key_dx<dim> move(const comb<dim> & c) const
	{
		grid_key_dx<dim> key = getKey();
		for (size_t i = 0 ; i < dim ; i++)
			key.set_d(i,key.get(i) + c[i]);
		return grid_dist_key_dx<dim>(getSub(),key);
	}

	/*! \brief Constructor set the sub-domain grid and the position in local coordinates
	 *
	 * \param g_c sub-domain
	 * \param key key
	 *
	 */
	inline grid_dist_key_dx(int g_c, const grid_key_dx<dim> & key)
	:g_c(g_c),key(key)
	{
	}

	//! Constructor
	inline grid_dist_key_dx(){}

	/*! \brief convert the key to string
	 *
	 *
	 */
	std::string to_string()
	{
		std::stringstream str;

		str << "sub_domain=" << g_c << " ";

		for (size_t i = 0 ; i < dim ; i++)
			str << "x[" << i << "]=" << key.get(i) << " ";

		str << "\n";

		return str.str();
	}
};

/*! \brief Distributed linearized key
 *
 *
 *
 */
class grid_dist_lin_dx
{
	//! grid list counter
	size_t g_c;

	//! Local grid iterator
	size_t key;

public:

	/*! \brief Set the local grid
	 *
	 * \param sub local grid
	 *
	 */
	inline void setSub(size_t sub)
	{
		g_c = sub;
	}

	/*! \brief Get the local grid
	 *
	 * \return the id of the local grid
	 *
	 */
	inline size_t getSub() const
	{
		return g_c;
	}

	/*! \brief Get the key
	 *
	 * \return the local key
	 *
	 */
	inline size_t getKey() const
	{
		return key;
	}


	/*! \brief Get the reference key
	 *
	 * \return the local key
	 *
	 */
	inline size_t & getKeyRef()
	{
		return key;
	}

	/* \brief Check if two key are the same
	 *
	 * \param key_t key to check
	 *
	 * \return true if the two key are equal
	 *
	 */

	inline bool operator==(const grid_dist_lin_dx & key_t)
	{
		if (g_c != key_t.g_c)
			return false;

		// Check the two key index by index

		return getKey() == key_t.getKey();
	}


	/*! \brief Constructor set the sub-domain grid and the position in local coordinates
	 *
	 * \param g_c sub-domain
	 * \param key key
	 *
	 */
	inline grid_dist_lin_dx(int g_c, size_t key)
	:g_c(g_c),key(key)
	{
	}

	//! Constructor
	inline grid_dist_lin_dx(){}

	/*! \brief convert the key to string
	 *
	 *
	 */
	std::string to_string()
	{
		std::stringstream str;

		str << "sub_domain=" << g_c << " ";
		str << "lin_id=" << key << " ";

		str << "\n";

		return str.str();
	}
};

/*! \brief Distributed linearized key
 *
 * instead of having the sub-subdomain index it store directly a pointer to the grid
 *
 */
template<typename device_grid>
class grid_dist_g_dx
{
	//! grid list counter
	device_grid * dg;

	//! Local grid iterator
	size_t key;

public:

	/*! \brief return the sub-domain grid
	 *
	 *
	 */
	inline device_grid * getSub()
	{
		return dg;
	}


	/*! \brief Get the key
	 *
	 * \return the local key
	 *
	 */
	inline size_t getKey() const
	{
		return key;
	}


	/*! \brief Get the reference key
	 *
	 * \return the local key
	 *
	 */
	inline size_t & getKeyRef()
	{
		return key;
	}

	/* \brief Check if two key are the same
	 *
	 * \param key_t key to check
	 *
	 * \return true if the two key are equal
	 *
	 */

	inline bool operator==(const grid_dist_g_dx & key_t)
	{
		if (dg != key_t.dg)
			return false;

		// Check the two key index by index

		return getKey() == key_t.getKey();
	}


	/*! \brief Constructor
	 *
	 * \param dg array of local grid
	 * \param key actual position linearized
	 *
	 */
	inline grid_dist_g_dx(device_grid * dg, size_t key)
	:dg(dg),key(key)
	{
	}

	//! Constructor
	inline grid_dist_g_dx(){}

	/*! \brief convert the key to string
	 *
	 * \return a string representing the position
	 *
	 */
	std::string to_string()
	{
		std::stringstream str;

		str << "sub_domain=" << dg << " ";
		str << "lin_id=" << key << " ";

		str << "\n";

		return str.str();
	}
};

#endif

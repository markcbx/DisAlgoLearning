#ifndef __SINGETON_HEADER__
#define __SINGETON_HEADER__


template <typename T>
class Singleton
{
public:
	template<typename... Args>
	static T* Tnstance(Args&&... args)
	{
		if (m_pInstance == nullptr)
		{
			m_pInstance = new T(std::forward<Args>(args)...);
			return m_pInstance;
		}
		return m_pInstance;
	}
	static T* GetInstance()
	{
		if (m_pInstance == nullptr)
		{
			throw std::logic_error("the object not create and init");
		}
		return m_pInstance;
	}
	static void DestroyInstance()
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
private:
	Singleton();
	virtual ~Singleton();
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton& param);

private:
	static T* m_pInstance;
};


template <class T> T* Singleton<T>::m_pInstance = nullptr;



#endif //__SINGETON_HEADER__

#ifndef QUEUE_HPP_
#define QUEUE_HPP_

template<typename T, size_t bufsize>
class queue {
public:
	queue() : siz(0), read(0), write(0) {}

	void push(const T &val){
		buf[write] = val;
		siz++;
		write++;
		if(write == bufsize)
			write = 0;
	}

	auto front() const -> const T& {
		return buf[read];
	}

	auto pop() -> const T {
		const auto ret = buf[read];
		siz--;
		read++;
		if(read == bufsize)
			read = 0;
		return ret;
	}

	auto empty() const -> const bool {
		return (siz == 0);
	}

	auto size() const -> const size_t {
		return siz;
	}
protected:
	size_t siz;
	size_t read, write;
	T buf[bufsize];
};

#endif

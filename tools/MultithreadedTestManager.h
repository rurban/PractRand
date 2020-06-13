class MultithreadedTestManager : public TestManager {
	class TestThread {
		Tests::TestBlock *base_block;
		Tests::TestBaseclass *test;
		Uint64 numblocks;
		Threading::Lock lock;
		bool finished;
	public:
		volatile bool retire() {
			lock.enter();
			if (!finished) {
				//must have tried to retire it too quickly to initialize properly
				lock.leave();
				Threading::sleep(0);
				return false;
			}
			delete this;
			return true;
		}
		TestThread(Tests::TestBaseclass *test_, Tests::TestBlock * base_block_, Uint64 numblocks_)
		:
			base_block(base_block_),
			test(test_),
			numblocks(numblocks_),
			finished(false)
		{
			//_thread_func(this);
			Threading::create_thread(_thread_func, this);
		}
		static THREADFUNC_RETURN_TYPE THREADFUNC_CALLING_CONVENTION _thread_func(void *param_) {
			TestThread *param = (TestThread*)param_;
			param->lock.enter();
			enum {MAX_BLOCKS_PER_CALL = 1ull << 18};
			while (param->numblocks > MAX_BLOCKS_PER_CALL) {
				param->test->test_blocks(param->base_block,MAX_BLOCKS_PER_CALL);
				param->numblocks -= MAX_BLOCKS_PER_CALL;
				param->base_block += MAX_BLOCKS_PER_CALL;
			}
			if (param->numblocks) param->test->test_blocks(param->base_block,(int)param->numblocks);
			param->finished = true;
			param->lock.leave();
			return 0;
		}
	};
	std::vector<TestThread *> threads;
	void wait_on_threads() {
		while(!threads.empty()) {
			TestThread *last = threads.back();
			if (last->retire()) threads.pop_back();
		}
	}


	/*std::vector<void*> work_units;
	void *(*issue_work_unit)(void (*work_func)(void *param));//work_unit is responsible for calling std::free on param when it retires
	bool (*retire_work_unit)(void *handle, void **param);//returns true on success
	void retire_all_work_units() {
		while(!work_units.empty()) {
			void *last = work_units.back();
			void *param;
			if (retire_work_unit(last, &param)) {
				threads.pop_back();
				std::free(param);
			}
		}
	}*/
public:
	//std::vector<Tests::TestBlock> buffer1, buffer2;
	std::vector<Tests::TestBlock> alt_buffer;
	//RNGs::vRNG *rng;
	//RNGs::vRNG *known_good;
	//Tests::ListOfTests *tests;
	//int max_buffer_amount;
	//int prefix_blocks;
	//int main_blocks;
	//Uint64 blocks_so_far;
	void multithreaded_prep_blocks(Uint64 num_blocks) {
		int num_prefix_blocks = Tests::TestBaseclass::REPEATED_BLOCKS;
		if (num_prefix_blocks > prefix_blocks + main_blocks)
			num_prefix_blocks = prefix_blocks + main_blocks;
		if (num_prefix_blocks) {
			memcpy(
				&alt_buffer[0], 
				&buffer[prefix_blocks + main_blocks - num_prefix_blocks], 
				Tests::TestBlock::SIZE * num_prefix_blocks
			);
		}
		prefix_blocks = num_prefix_blocks;
		main_blocks = (num_blocks > max_buffer_amount) ? max_buffer_amount : Uint32(num_blocks);
		buffer[prefix_blocks].fill(rng, main_blocks);
		blocks_so_far += main_blocks;
	}
public:
	MultithreadedTestManager(RNGs::vRNG *rng_, Tests::ListOfTests *tests_, RNGs::vRNG *known_good_, int max_buffer_amount_ = 1 << (27-10)) : TestManager(rng_, tests_, known_good_, max_buffer_amount_) {
		//buffer1.resize(max_buffer_amount + Tests::TestBaseclass::REPEATED_BLOCKS);
		alt_buffer.resize(max_buffer_amount + Tests::TestBaseclass::REPEATED_BLOCKS);
		for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->init(known_good);
	}
	void test(Uint64 num_blocks) {
		while (num_blocks) {
			multithreaded_prep_blocks(num_blocks);
			num_blocks -= main_blocks;
			wait_on_threads();
			alt_buffer.swap(buffer);
			for (unsigned int i = 0; i < tests->tests.size(); i++) {
				threads.push_back( new TestThread( tests->tests[i], &alt_buffer[prefix_blocks], main_blocks ) );
			}
		}
		wait_on_threads();
	}
	virtual void reset(RNGs::vRNG *rng_ = NULL) {//resets contents for starting a new test run ; if rng is NULL then it will reuse the current RNG
		for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->deinit();
		for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->init(known_good);
		if (rng_) rng = rng_;
		main_blocks = 0;
		prefix_blocks = 0;
		blocks_so_far = 0;
	}
};

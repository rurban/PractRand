class TestManager {
protected:
	std::vector<Tests::TestBlock> buffer;
	RNGs::vRNG *rng;
	RNGs::vRNG *known_good;
	Tests::ListOfTests *tests;
	int max_buffer_amount;
	int prefix_blocks;
	int main_blocks;
	Uint64 blocks_so_far;
	int prep_blocks(Uint64 &blocks);
public:
	const RNGs::vRNG *get_rng() const {return rng;}
	Uint64 get_blocks_so_far() {return blocks_so_far;}

	TestManager(RNGs::vRNG *rng_, Tests::ListOfTests *tests_, RNGs::vRNG *known_good_, int max_buffer_amount_ = 1 << (25-10));

	virtual ~TestManager();//destructor (destroys the tests in the ListOfTests)

	virtual void reset(RNGs::vRNG *rng_ = NULL);//resets contents for starting a new test run ; if rng is NULL then it will reuse the current RNG

	virtual void test(Uint64 blocks);//does testing... the number of blocks is ADDITIONAL blocks to test, not total blocks to test

	virtual void get_results( std::vector<TestResult> &result_vec );//gets the results
};

TestManager::TestManager(RNGs::vRNG *rng_, Tests::ListOfTests *tests_, RNGs::vRNG *known_good_, int max_buffer_amount_) {
	rng = rng_;
	tests = tests_;
	known_good = known_good_;
	blocks_so_far = 0;
	max_buffer_amount = max_buffer_amount_;
	prefix_blocks = 0;
	main_blocks = 0;
	buffer.resize(max_buffer_amount + Tests::TestBaseclass::REPEATED_BLOCKS);
	for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->init(known_good);
}
TestManager::~TestManager() {
	for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->deinit();
	for (unsigned int i = 0; i < tests->tests.size(); i++) delete tests->tests[i];
}
void TestManager::reset(RNGs::vRNG *rng_) {
	for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->deinit();
	for (unsigned int i = 0; i < tests->tests.size(); i++) tests->tests[i]->init(known_good);
	if (rng_) rng = rng_;
	main_blocks = 0;
	prefix_blocks = 0;
	blocks_so_far = 0;
}
int TestManager::prep_blocks(Uint64 &blocks) {
	Uint64 _delta_blocks = blocks;
	if (_delta_blocks > max_buffer_amount) _delta_blocks = max_buffer_amount;
	int delta_blocks = int(_delta_blocks);
	blocks -= delta_blocks;
	size_t repeat_region_start, repeat_region_size;
	if (prefix_blocks + main_blocks >= Tests::TestBaseclass::REPEATED_BLOCKS) {
		repeat_region_start = prefix_blocks + main_blocks - Tests::TestBaseclass::REPEATED_BLOCKS;
		repeat_region_size = Tests::TestBaseclass::REPEATED_BLOCKS;
	}
	else {
		repeat_region_start = 0;
		repeat_region_size = prefix_blocks + main_blocks;
	}
	if (repeat_region_start != 0)
		std::memmove(&buffer[0], &buffer[repeat_region_start], repeat_region_size * Tests::TestBlock::SIZE);
	prefix_blocks = repeat_region_size;
	main_blocks = delta_blocks;
	buffer[prefix_blocks].fill(rng, main_blocks);
	blocks_so_far += delta_blocks;
	return delta_blocks;
}
void TestManager::test(Uint64 num_blocks) {
	while (num_blocks) {
		prep_blocks(num_blocks);
		for (unsigned int i = 0; i < tests->tests.size(); i++)
			tests->tests[i]->test_blocks(&buffer[prefix_blocks], main_blocks);
	}
}
void TestManager::get_results( std::vector<TestResult> &result_vec ) {
	for (unsigned int i = 0; i < tests->tests.size(); i++) {
		tests->tests[i]->get_results(result_vec);
	}
}

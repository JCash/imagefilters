
#include <stdio.h>
#include <unistd.h>

#define JC_TEST_IMPLEMENTATION
#include "jc_test.h"

#define JC_INI_IMPLEMENTATION
#include "../src/jc_ini.h"

#define JC_IMAGEFILTERS_IMPLEMENTATION
#include "../src/jc_imagefilters.h"

static void* test_fixture_setup()
{
    return 0;
}

static void test_fixture_teardown()
{   
}

static void test_setup()
{   
}

static void test_teardown()
{   
}

static void test_getline()
{
    const char* path = "getline.txt";
    FILE* f = fopen(path, "wb");
    ASSERT_TRUE( f != 0 );

    // line 1
    for( int i = 0; i < 32; ++i)
        fputc('A' + i%25, f);
    fputc('\n', f);

    // line 2
    for( int i = 0; i < (BUFSIZ*3)/2; ++i)
        fputc('A' + i%25, f);
    fputc('\n', f);

    fclose(f);

    //

    size_t size = 16;
    char* line = (char*)malloc(size);

    f = fopen("getline.txt", "rb");
    ASSERT_TRUE( f != 0 );

    line = jc_ini_getline(f, &size, line);

    size_t linesize = 32;
    ASSERT_EQ( linesize+1, strlen(line) );
    for( int i = 0; i < (int)linesize; ++i)
    {
        ASSERT_EQ( 'A' + i%25, line[i] );
    }
    ASSERT_EQ( '\n', line[linesize] );


    line = jc_ini_getline(f, &size, line);

    linesize = (BUFSIZ*3)/2;
    ASSERT_EQ( linesize+1, strlen(line) );
    for( int i = 0; i < (int)linesize; ++i)
    {
        ASSERT_EQ( 'A' + i%25, line[i] );
    }
    ASSERT_EQ( '\n', line[linesize] );

    fclose(f);

    free(line);

    unlink(path);
}

static void Write(const char* str, FILE* f)
{
    size_t len = strlen(str);
    fwrite(str, 1, len, f);
}


static void load_handler1(const char* section, const char* key, const char* value, void* ctx)
{
    int* num_ok = (int*)ctx;

    //printf("HANDLER: '%s'  '%s'  '%s'\n", section ? section : "null", key ? key : "null", value ? value : "null");

#define MATCH_PTR(_EXPECTED, _PTR) \
    if( ((_EXPECTED) == (_PTR)) || ((_EXPECTED) && (_PTR) && strcmp(_EXPECTED, _PTR) == 0) ) matches++;

#define VERIFY_SKV(_S, _K, _V) \
    { \
        int matches = 0; \
        MATCH_PTR( _S, section ); \
        MATCH_PTR( _K, key ); \
        MATCH_PTR( _V, value ); \
        if( matches == 3 ) (*num_ok)++; \
    }

    VERIFY_SKV(0, "a", "1");
    VERIFY_SKV(0, "b", "2");
    VERIFY_SKV(0, "a", "2");
    VERIFY_SKV("section 1", "c", "\"text\"");
    VERIFY_SKV("section 1", "d", "[1,2,3]");
    VERIFY_SKV("section 2", "e", "10");
    VERIFY_SKV("section 2", "f", "11");
    VERIFY_SKV("section 2", "g", "");

#undef VERIFY_SKV

}

static void test_load_ini_ok()
{
    const char* path = "test.cfg";
    FILE* f = fopen(path, "wb");
    ASSERT_TRUE( f != 0 );
    Write("; comment 1\n", f);
    Write("# comment 2\n", f);
    Write("  ; comment 3\n", f);
    Write("a = 1\n", f);
    Write("b = 2\n", f);
    Write("a = 2\n", f);
    Write("\n", f);
    Write("[section 1]\n", f);
    Write("c=\"text\"\n", f);
    Write("d = [1,2,3]\n", f);
    Write("\n", f);
    Write("[section 2] ; comment\n", f);
    Write("e = 10 ; comment\n", f);
    Write("  f = 11 ; indentation\n", f);
    Write("g =\n", f);
    fclose(f);

    //
    int num_ok = 0;
    int result = jc_ini_parse_from_path(path, load_handler1, &num_ok);
    ASSERT_EQ( 0, result );
    ASSERT_EQ( 8, num_ok );

    unlink(path);
}


static void test_map_curve()
{
    uint8_t pts[] = { 25, 50, 150, 250 };
    uint32_t npoints = sizeof(pts)/sizeof(pts[0]);

    int value = 0;
    value = (int)(jc_if_apply_curve_value(pts, npoints, 0));
    
    printf("\nV: %u\n", value);
    ASSERT_EQ( 25, value );

    printf("\n");

    value = (int)(jc_if_apply_curve_value(pts, npoints, 128));
    
    printf("\nV: %u\n", value);
    ASSERT_EQ( 100, value );
    
    printf("\n");
}

TEST_BEGIN(ImageFilters, test_fixture_setup, test_fixture_teardown, test_setup, test_teardown)
    TEST(test_getline)
    TEST(test_load_ini_ok)
    TEST(test_map_curve)
TEST_END(ImageFilters)


int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;
    return TEST_RUN_ALL();
}

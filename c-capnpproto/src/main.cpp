#include "capnp_c.h"
#include "addressbook.capnp.h"

int main() {

    struct capn c;
    capn_init_malloc(&c);
    capn_ptr cr = capn_root(&c);
    struct capn_segment *cs = cr.seg;

    // Set initial object in `p`.
    struct Person p = {
        .id = 17,
        .name = chars_to_text(name),
        .email = chars_to_text(email),
    };
    p.employment_which = Person_employment_school;
    p.employment.school = chars_to_text(school);

    p.phones = new_Person_PhoneNumber_list(cs, 2);
    struct Person_PhoneNumber pn0 = {
        .number = chars_to_text("123"),
        .type = Person_PhoneNumber_Type_work,
    };
    set_Person_PhoneNumber(&pn0, p.phones, 0);
    struct Person_PhoneNumber pn1 = {
        .number = chars_to_text("234"),
        .type = Person_PhoneNumber_Type_home,
    };
    set_Person_PhoneNumber(&pn1, p.phones, 1);

    Person_ptr pp = new_Person(cs);
    write_Person(&p, pp);
    int setp_ret = capn_setp(capn_root(&c), 0, pp.p);
    ASSERT_EQ(0, setp_ret);
    sz = capn_write_mem(&c, buf, sizeof(buf), 0 /* packed */);
    capn_free(&c);
}
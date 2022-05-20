#include <stdio.h>

class Person
{
    public: virtual void ask();
};

class GoodPerson : public Person
{
    public: void ask()
    {
        puts("test goodie");
    }
};

class EvilPerson : public Person
{
    public: void ask()
    {
        puts("test bad");
    }
};

extern "C" void askOpinion()
{
    Person *tPerson;

    tPerson=new GoodPerson();
    tPerson->ask();
    delete(tPerson);

    tPerson=new EvilPerson();
    tPerson->ask();
    delete(tPerson);
}
#include "gqf.h"
#include "bufferedMQF.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>
#include<iostream>
#include "catch.hpp"
#include <unordered_map>
using namespace std;



const uint64_t MemSize=50;

TEST_CASE( "simple counting test(buffered)","[buffered]" ) {
  //except first item is inserted 5 times to full test _insert1
  bufferedMQF qf;
  int counter_size=2;
  uint64_t qbits=5;
  uint64_t diskQbits=6;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  uint64_t count,fixed_counter;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(&qf ,(1ULL<<qbits),(1ULL<<diskQbits) , num_hash_bits, 0,counter_size, "tmp.ser");



  for(uint64_t i=0;i<=10;i++){
    bufferedMQF_insert(&qf,100,1,false,false);
    count = bufferedMQF_count_key(&qf, 100);
    //fixed_counter=qf_get_fixed_counter(&qf,100);
    INFO("Counter = "<<count<<" fixed counter = "<<fixed_counter)
    CHECK(count == (1+i));
  }


  bufferedMQF_insert(&qf,1500,50,false,false);

  count = bufferedMQF_count_key(&qf, 1500);
  //  fixed_counter=qf_get_fixed_counter(&qf,1500);
  INFO("Counter = "<<count<<" fixed counter = "<<fixed_counter)
  CHECK(count == (50));

  bufferedMQF_insert(&qf,1600,60,false,false);
  count = bufferedMQF_count_key(&qf, 1600);
  //  fixed_counter=qf_get_fixed_counter(&qf,1600);
  INFO("Counter = "<<count<<" fixed counter = "<<fixed_counter)
  CHECK(count == (60));


  bufferedMQF_insert(&qf,2000,4000,false,false);
  count = bufferedMQF_count_key(&qf, 2000);
  //  fixed_counter=qf_get_fixed_counter(&qf,2000);
  INFO("Counter = "<<count<<" fixed counter = "<<fixed_counter)
  CHECK(count == (4000));

}

TEST_CASE( "Maximum count(buffered)","[buffered]" ) {
  bufferedMQF qf;
  int counter_size=4;
  srand (1);
  uint64_t qbits=5;
  uint64_t diskQbits=6;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(&qf,(1ULL<<diskQbits), (1ULL<<qbits), num_hash_bits, 0,counter_size, "tmp.ser");
  qf.memoryBuffer->metadata->maximum_count=10;
  qf.disk->metadata->maximum_count=10;
  bufferedMQF_insert(&qf,100,100000,false,false);
  uint64_t count = bufferedMQF_count_key(&qf, 100);
  CHECK(count==10);

  bufferedMQF_insert(&qf,150,8,false,false);
  bufferedMQF_insert(&qf,150,8,false,false);
  count = bufferedMQF_count_key(&qf, 150);
  CHECK(count==10);

}
//
TEST_CASE( "Big count(buffered)","[buffered]" ) {
  bufferedMQF qf;
  int counter_size=4;
  srand (1);
  uint64_t qbits=5;
  uint64_t diskQbits=6;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(&qf,(1ULL<<diskQbits), (1ULL<<qbits), num_hash_bits, 0,counter_size, "tmp.ser");
  bufferedMQF_insert(&qf,100,100000,false,false);
  uint64_t count = bufferedMQF_count_key(&qf, 100);

  CHECK(count==100000);

}

TEST_CASE( "Inserting items( repeated 1 time) in cqf(90% load factor )(buffered)" ,"[buffered]") {
  //except first item is inserted 5 times to full test _insert1
  bufferedMQF* qf;
  qf=new bufferedMQF();
  int counter_size=2;
  uint64_t qbits=15;
  uint64_t diskQbits=16;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.ser");

  uint64_t nvals = (1ULL<<diskQbits)*2;
  uint64_t *vals;
  vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
  for(uint64_t i=0;i<nvals;i++)
  {
    vals[i]=rand();
    vals[i]=(vals[i]<<32)|rand();
    vals[i]=vals[i]%(qf->disk->metadata->range);
  }

  int loadFactor=bufferedMQF_space(qf);;
  uint64_t insertedItems=0;

    bufferedMQF_insert(qf,vals[0],1,false,false);
    bufferedMQF_insert(qf,vals[0],1,false,false);
    bufferedMQF_insert(qf,vals[0],1,false,false);
    bufferedMQF_insert(qf,vals[0],1,false,false);
  // for(uint64_t i=0;i<32;i++)
  // {
  //   cout<<get_fixed_counter(&qf,i)<<"-";
  // }
  //cout<<endl;
  while(loadFactor<70){

    bufferedMQF_insert(qf,vals[insertedItems],1,false,false);
    // for(uint64_t i=0;i<32;i++)
    // {
    //   cout<<get_fixed_counter(&qf,i)<<"-";
    // }
    // cout<<endl;
    insertedItems++;
    loadFactor=bufferedMQF_space(qf);

  }
  INFO("Inserted Items = "<<insertedItems);

  uint64_t count;
  //INFO("Fixed counter = "<<qf_get_fixed_counter(&qf,vals[0]));
  count = bufferedMQF_count_key(qf, vals[0]);
  CHECK(count >= 5);

  for(uint64_t i=1;i<insertedItems;i++)
  {
    count = bufferedMQF_count_key(qf, vals[i]);
    CHECK(count >= 1);
  }
  bufferedMQFIterator* qfi= bufferedMQF_iterator(qf, 0);
  do {
    uint64_t key, value, count;
    qfi->get(&key, &value, &count);

    if(key==vals[0]){
      REQUIRE(count >= 5);
    }
    else{
      REQUIRE(count >= 1);
    }

  } while(!qfi->next());

// bufferedMQF_destroy(qf);
  delete qf;
}

TEST_CASE( "batch query(singletons)" ,"[buffered]") {
  //except first item is inserted 5 times to full test _insert1
  bufferedMQF qf;
  int counter_size=2;
  uint64_t qbits=15;

  uint64_t diskQbits=17;
  uint64_t num_hash_bits=qbits+9;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);

  bufferedMQF_init(&qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.ser");

  uint64_t nvals = (1ULL<<diskQbits)*2;
  uint64_t *vals;
  vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
  for(uint64_t i=0;i<nvals;i++)
  {
    vals[i]=rand();
    vals[i]=(vals[i]<<32)|rand();
    vals[i]=vals[i]%(qf.disk->metadata->range);
  }

  int loadFactor=bufferedMQF_space(&qf);;
  uint64_t insertedItems=0;

    bufferedMQF_insert(&qf,vals[0],1,false,false);
    bufferedMQF_insert(&qf,vals[0],1,false,false);
    bufferedMQF_insert(&qf,vals[0],1,false,false);
    bufferedMQF_insert(&qf,vals[0],1,false,false);
  // for(uint64_t i=0;i<32;i++)
  // {
  //   cout<<get_fixed_counter(&qf,i)<<"-";
  // }
  //cout<<endl;
  while(loadFactor<80){

    bufferedMQF_insert(&qf,vals[insertedItems],1,false,false);
    // for(uint64_t i=0;i<32;i++)
    // {
    //   cout<<get_fixed_counter(&qf,i)<<"-";
    // }
    // cout<<endl;
    insertedItems++;
    loadFactor=bufferedMQF_space(&qf);

  }
  INFO("Inserted Items = "<<insertedItems);

  uint64_t count;
  //INFO("Fixed counter = "<<qf_get_fixed_counter(&qf,vals[0]));
  count = bufferedMQF_count_key(&qf, vals[0]);
  CHECK(count >= 5);
  QF *inputBuffer,*outputBuffer;
  inputBuffer=new QF();
  outputBuffer=new QF();
  qf_init(inputBuffer, (1ULL<<qbits), num_hash_bits, 0,2, 0,true, "", 2038074761);
  qf_init(outputBuffer, (1ULL<<qbits), num_hash_bits, 0,2,0,true, "", 2038074761);
  uint64_t i=1;
  uint64_t start=1;
  for(;i<insertedItems;i++)
  {
    qf_insert(inputBuffer,vals[i],1);
    if(qf_space(inputBuffer)>70)
    {
      bufferedMQF_BatchQuery(&qf,inputBuffer);
      for(int j=start;j<=i;j++)
      {
        count=qf_count_key(inputBuffer,vals[j]);
        CHECK(count >= 1);
      }
      start=i+1;
      qf_reset(inputBuffer);
      qf_reset(outputBuffer);
    }
    count = bufferedMQF_count_key(&qf, vals[i]);
  }
  bufferedMQF_BatchQuery(&qf,inputBuffer);
  for(int j=start;j<i;j++)
  {
    count=qf_count_key(inputBuffer,vals[j]);
    CHECK(count >= 1);
  }

  // bufferedMQFIterator qfi;
  // bufferedMQF_iterator(&qf, &qfi, 0);
  // do {
  //   uint64_t key, value, count;
  //   qfi_get(&qfi, &key, &value, &count);
  //   count=bufferedMQF_count_key(&qf, key);
  //   if(key==vals[0]){
  //     CHECK(count >= 5);
  //   }
  //   else{
  //     CHECK(count >= 1);
  //   }
  //
  // } while(!qfi_next(&qfi));


}

TEST_CASE( "Saving and loading(buffered)" ,"[buffered]") {
  //except first item is inserted 5 times to full test _insert1
  bufferedMQF* qf;
  qf=new bufferedMQF();
  int counter_size=2;
  uint64_t qbits=16;
  uint64_t diskQbits=18;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp1.ser");

  uint64_t nvals = (1ULL<<diskQbits)*2;
  uint64_t *vals;
  vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
  for(uint64_t i=0;i<nvals;i++)
  {
    vals[i]=rand();
    vals[i]=(vals[i]<<32)|rand();
    vals[i]=vals[i]%(qf->disk->metadata->range);
  }

  int loadFactor=bufferedMQF_space(qf);;
  uint64_t insertedItems=0;

  bufferedMQF_insert(qf,vals[0],1,false,false);
  bufferedMQF_insert(qf,vals[0],1,false,false);
  bufferedMQF_insert(qf,vals[0],1,false,false);
  bufferedMQF_insert(qf,vals[0],1,false,false);
  // for(uint64_t i=0;i<32;i++)
  // {
  //   cout<<get_fixed_counter(&qf,i)<<"-";
  // }
  //cout<<endl;
  while(loadFactor<70){

    bufferedMQF_insert(qf,vals[insertedItems],1,false,false);
    // for(uint64_t i=0;i<32;i++)
    // {
    //   cout<<get_fixed_counter(&qf,i)<<"-";
    // }
    // cout<<endl;
    insertedItems++;
    loadFactor=bufferedMQF_space(qf);

  }
  INFO("Inserted Items = "<<insertedItems);
  bufferedMQF_serialize(qf);
  delete qf;

  bufferedMQF* qf2=new bufferedMQF();
  bufferedMQF_deserialize(qf2,"tmp1.ser");
  uint64_t count;
  //INFO("Fixed counter = "<<qf_get_fixed_counter(&qf,vals[0]));
  count = bufferedMQF_count_key(qf2, vals[0]);
  CHECK(count >= 5);

  for(uint64_t i=1;i<insertedItems;i++)
  {
    count = bufferedMQF_count_key(qf2, vals[i]);
    CHECK(count >= 1);
  }
  bufferedMQFIterator* qfi= bufferedMQF_iterator(qf2, 0);
  do {
    uint64_t key, value, count;
    qfi->get(&key, &value, &count);

    if(key==vals[0]){
      REQUIRE(count >= 5);
    }
    else{
      REQUIRE(count >= 1);
    }

  } while(!qfi->next());

// bufferedMQF_destroy(qf);
  delete qf2;
}


TEST_CASE( "Saving and loading and deleteing memory buffer(buffered)" ,"[buffered]") {
  //except first item is inserted 5 times to full test _insert1
  bufferedMQF* qf;
  qf=new bufferedMQF();
  int counter_size=2;
  uint64_t qbits=16;
  uint64_t diskQbits=18;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.ser");

  uint64_t nvals = (1ULL<<diskQbits)*2;
  uint64_t *vals;
  vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
  for(uint64_t i=0;i<nvals;i++)
  {
    vals[i]=rand();
    vals[i]=(vals[i]<<32)|rand();
    vals[i]=vals[i]%(qf->disk->metadata->range);
  }

  int loadFactor=bufferedMQF_space(qf);;
  uint64_t insertedItems=0;

  bufferedMQF_insert(qf,vals[0],1,false,false);
  bufferedMQF_insert(qf,vals[0],1,false,false);
  bufferedMQF_insert(qf,vals[0],1,false,false);
  bufferedMQF_insert(qf,vals[0],1,false,false);
  // for(uint64_t i=0;i<32;i++)
  // {
  //   cout<<get_fixed_counter(&qf,i)<<"-";
  // }
  //cout<<endl;
  while(loadFactor<70){

    bufferedMQF_insert(qf,vals[insertedItems],1,false,false);
    // for(uint64_t i=0;i<32;i++)
    // {
    //   cout<<get_fixed_counter(&qf,i)<<"-";
    // }
    // cout<<endl;
    insertedItems++;
    loadFactor=bufferedMQF_space(qf);

  }
  INFO("Inserted Items = "<<insertedItems);
  bufferedMQF_serialize(qf);
  delete qf;

  bufferedMQF* qf2=new bufferedMQF();
  bufferedMQF_deserialize(qf2,"tmp.ser");
  bufferedMQF_deleteMemoryBuffer(qf2);
  uint64_t count;
  //INFO("Fixed counter = "<<qf_get_fixed_counter(&qf,vals[0]));
  count = bufferedMQF_count_key(qf2, vals[0]);
  CHECK(count >= 5);

  for(uint64_t i=1;i<insertedItems;i++)
  {
    count = bufferedMQF_count_key(qf2, vals[i]);
    CHECK(count >= 1);
  }
  bufferedMQFIterator* qfi= bufferedMQF_iterator(qf2, 0);
  do {
    uint64_t key, value, count;
    qfi->get(&key, &value, &count);

    if(key==vals[0]){
      REQUIRE(count >= 5);
    }
    else{
      REQUIRE(count >= 1);
    }

  } while(!qfi->next());

// bufferedMQF_destroy(qf);
  delete qf2;
}



//
//
// TEST_CASE( "Inserting items( repeated 50 times) in cqf(90% load factor )" ) {
//   bufferedMQF qf;
//   int counter_size=4;
//   uint64_t qbits=15;
//   uint64_t num_hash_bits=qbits+8;
//   uint64_t maximum_count=(1ULL<<counter_size)-1;
//   INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
//   bufferedMQF_init(&qf, (1ULL<<qbits), num_hash_bits, 0,counter_size, true, "", 2038074761);
//
//   uint64_t nvals = (1ULL<<qbits);
//   uint64_t *vals;
//   vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
//   for(uint64_t i=0;i<nvals;i++)
//   {
//     vals[i]=rand();
//     vals[i]=(vals[i]<<32)|rand();
//     vals[i]=vals[i]%(qf.metadata->range);
//   }
//   double loadFactor=(double)qf.metadata->noccupied_slots/(double)qf.metadata->nslots;
//   uint64_t insertedItems=0;
//   uint64_t count;
//   while(loadFactor<0.9){
//     bufferedMQF_insert(&qf,vals[insertedItems],50,false,false);
//
//     insertedItems++;
//     loadFactor=(double)qf.metadata->noccupied_slots/(double)qf.metadata->nslots;
//   }
//
//   for(uint64_t i=0;i<insertedItems;i++)
//   {
//     count = bufferedMQF_count_key(&qf, vals[i]);
//     CHECK(count >= 50);
//   }
//   bufferedMQFIterator qfi;
//   bufferedMQF_iterator(&qf, &qfi, 0);
//   do {
//     uint64_t key, value, count;
//     qfi_get(&qfi, &key, &value, &count);
//     count=bufferedMQF_count_key(&qf, key);
//     CHECK(count >= 50);
//   } while(!qfi_next(&qfi));
//

//
// }
//
//
TEST_CASE( "Inserting items( repeated 1-1000 times) in cqf(90% load factor )(buffered)","[buffered]" ) {
  bufferedMQF qf;
  int counter_size=4;
  srand (1);
  uint64_t qbits=15;
  uint64_t diskQbits=16;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(&qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.ser");

  uint64_t nvals = (1ULL<<diskQbits);
  //uint64_t nvals = 3;
  uint64_t *vals;
  uint64_t *nRepetitions;
  vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
  nRepetitions= (uint64_t*)malloc(nvals*sizeof(nRepetitions[0]));
  uint64_t count;
  unordered_map<uint64_t,uint64_t> gold;

  for(uint64_t i=0;i<nvals;i++)
  {
    uint64_t newvalue=0;
    while(newvalue==0){
      newvalue=rand();
      newvalue=(newvalue<<32)|rand();
      newvalue=newvalue%(qf.memoryBuffer->metadata->range);
      for(uint64_t j=0;j<i;j++)
      {
        if(vals[j]==newvalue)
        {
          newvalue=0;
          break;
        }
      }
    }
    vals[i]=newvalue;


    nRepetitions[i]=(rand()%1000)+1;
  }
  int loadFactor=bufferedMQF_space(&qf);
  uint64_t insertedItems=0;
  while(insertedItems<nvals && loadFactor<70){
  //  printf("inserting %lu count = %lu\n",vals[insertedItems],nRepetitions[insertedItems] );
    INFO("Inserting "<< vals[insertedItems] << " Repeated "<<nRepetitions[insertedItems]);
    bufferedMQF_insert(&qf,vals[insertedItems],nRepetitions[insertedItems],false,false);
    gold[vals[insertedItems]]=nRepetitions[insertedItems];
    //qf_dump(&qf);
    INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);
 //   count = bufferedMQF_count_key(&qf, vals[insertedItems]);

  //  CHECK(count == nRepetitions[insertedItems]);
    insertedItems++;
    loadFactor=bufferedMQF_space(&qf);

  }
  INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);

//  for(uint64_t i=0;i<insertedItems;i++)
//  {
//    count = bufferedMQF_count_key(&qf, vals[i]);
//    INFO("value = "<<vals[i]<<" Repeated " <<nRepetitions[i]);
//    CHECK(count == nRepetitions[i]);
//      CHECK(gold[vals[i]] == nRepetitions[i]);
//  }
 // bufferedMQF_syncBuffer(&qf);

  bufferedMQFIterator* qfi= bufferedMQF_iterator(&qf, 0);
  while(!qfi->end()) {
    uint64_t key, value, count;
    qfi->get(&key, &value, &count);
    REQUIRE(count == gold[key]);
    //gold.erase(key);
    if(key==vals[0]){
      REQUIRE(count >= 5);
    }
    else{
      REQUIRE(count >= 1);
    }
    qfi->next();

  }
  //REQUIRE(gold.size()==0);


}
TEST_CASE( "batch query" ,"[buffered]"){
  bufferedMQF qf;
  int counter_size=4;
  srand (1);
  uint64_t qbits=15;
  uint64_t diskQbits=16;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(&qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.ser");

  uint64_t nvals = (1ULL<<diskQbits);
  //uint64_t nvals = 3;
  uint64_t *vals;
  uint64_t *nRepetitions;
  vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
  nRepetitions= (uint64_t*)malloc(nvals*sizeof(nRepetitions[0]));
  uint64_t count;

  for(uint64_t i=0;i<nvals;i++)
  {
    uint64_t newvalue=0;
    while(newvalue==0){
      newvalue=rand();
      newvalue=(newvalue<<32)|rand();
      newvalue=newvalue%(qf.memoryBuffer->metadata->range);
      for(uint64_t j=0;j<i;j++)
      {
        if(vals[j]==newvalue)
        {
          newvalue=0;
          break;
        }
      }
    }
    vals[i]=newvalue;


    nRepetitions[i]=(rand()%1000)+1;
  }
  int loadFactor=bufferedMQF_space(&qf);
  uint64_t insertedItems=0;
  while(insertedItems<nvals && loadFactor<60){
  //  printf("inserting %lu count = %lu\n",vals[insertedItems],nRepetitions[insertedItems] );
    INFO("Inserting "<< vals[insertedItems] << " Repeated "<<nRepetitions[insertedItems]);
    bufferedMQF_insert(&qf,vals[insertedItems],nRepetitions[insertedItems],false,false);
    //qf_dump(&qf);
    INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);
    count = bufferedMQF_count_key(&qf, vals[insertedItems]);
    CHECK(count == nRepetitions[insertedItems]);
    insertedItems++;
    loadFactor=bufferedMQF_space(&qf);

  }
  INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);

  QF *inputBuffer;
  inputBuffer=new QF();
  qf_init(inputBuffer, (1ULL<<qbits+1), num_hash_bits, 0,2, 0,true, "", 2038074761);





  uint64_t i=1;
  uint64_t start=1;
  for(;i<insertedItems;i++)
  {
    qf_insert(inputBuffer,vals[i],1);
    if(qf_space(inputBuffer)>50)
    {
      bufferedMQF_BatchQuery(&qf,inputBuffer);
      for(int j=start;j<=i;j++)
      {
        count=qf_count_key(inputBuffer,vals[j]);
        INFO("value = "<<vals[j]<<" Repeated " <<nRepetitions[j]);
        REQUIRE(count >= nRepetitions[j]);
      }
      start=i+1;
      qf_reset(inputBuffer);
    }

  }
  bufferedMQF_BatchQuery(&qf,inputBuffer);
  for(int j=start;j<i;j++)
  {
    count=qf_count_key(inputBuffer,vals[j]);
    INFO("value = "<<vals[j]<<" Repeated " <<nRepetitions[j]);
    CHECK(count >= nRepetitions[j]);
  }



}

//
 TEST_CASE( "Migrate(buffered)" ,"[buffered]" ) {
   bufferedMQF *qf,*qf2;
   qf=new bufferedMQF();
   qf2=new bufferedMQF();
   int counter_size=4;
   srand (1);
   uint64_t qbits=15;
   uint64_t diskQbits=16;
   uint64_t num_hash_bits=qbits+8;
   uint64_t maximum_count=(1ULL<<counter_size)-1;
   INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
   bufferedMQF_init(qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.buffered.ser");
   bufferedMQF_init(qf2 ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.buffered.ser2");


   uint64_t nvals = (1ULL<<qbits);
   //uint64_t nvals = 3;
   uint64_t *vals;
   uint64_t *nRepetitions;
   vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
   nRepetitions= (uint64_t*)malloc(nvals*sizeof(nRepetitions[0]));
   uint64_t count;

   for(uint64_t i=0;i<nvals;i++)
   {
     uint64_t newvalue=0;
     while(newvalue==0){
       newvalue=rand();
       newvalue=(newvalue<<32)|rand();
       newvalue=newvalue%(qf->disk->metadata->range);
       for(uint64_t j=0;j<i;j++)
       {
         if(vals[j]==newvalue)
         {
           newvalue=0;
           break;
         }
       }
     }
     vals[i]=newvalue;


     nRepetitions[i]=(rand()%1000)+1;
   }

   double loadFactor=(double)qf2->disk->metadata->noccupied_slots/(double)qf2->disk->metadata->nslots;
   uint64_t insertedItems=0;
   while(insertedItems<nvals && loadFactor<0.6){
   //  printf("inserting %lu count = %lu\n",vals[insertedItems],nRepetitions[insertedItems] );

     INFO("Inserting "<< vals[insertedItems] << " Repeated "<<nRepetitions[insertedItems]);
     try {
         bufferedMQF_insert(qf2, vals[insertedItems], nRepetitions[insertedItems], false, false);
     }
     catch(exception& e)
     {
         cout<<" on test " << e.what() << '\n';
     }
     //  bufferedMQF_insert(qf,vals[insertedItems],nRepetitions[insertedItems],false,false);
     //qf_dump(&qf);
     INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems<<" last value= "<<vals[insertedItems]);
     count = bufferedMQF_count_key(qf2, vals[insertedItems]);
   //  CHECK(count == nRepetitions[insertedItems]);
     insertedItems++;
     loadFactor=(double)qf2->disk->metadata->noccupied_slots/(double)qf2->disk->metadata->nslots;

   }
   INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);
    //bufferedMQF_syncBuffer(qf2);
    bufferedMQF_migrate(qf2,qf);
   //bufferedMQF_syncBuffer(qf);
   for(uint64_t i=0;i<insertedItems;i++)
   {
     count = bufferedMQF_count_key(qf, vals[i]);
     INFO("value = "<<vals[i]<<" Repeated " <<nRepetitions[i]);
     CHECK(count == nRepetitions[i]);
   }

   free(vals);
   free(nRepetitions);
   delete qf ,qf2;
   //bufferedMQF_destroy(qf);
   //bufferedMQF_destroy(qf2);



 }

TEST_CASE( "test get_iterator(buffered)" ,"[buffered]"  ) {
    bufferedMQF qf;
    int counter_size=4;
    srand (1);
    uint64_t qbits=15;
    uint64_t diskQbits=16;
    uint64_t num_hash_bits=qbits+8;
    uint64_t maximum_count=(1ULL<<counter_size)-1;
    INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
    bufferedMQF_init(&qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.ser");

    uint64_t nvals = (1ULL<<qbits);
    //nvals = 5000;
    uint64_t *vals;
    uint64_t *nRepetitions;
    vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
    nRepetitions= (uint64_t*)malloc(nvals*sizeof(nRepetitions[0]));
    uint64_t count;

    for(uint64_t i=0;i<nvals;i++)
    {
        uint64_t newvalue=0;
        while(newvalue==0){
            newvalue=rand();
            newvalue=(newvalue<<32)|rand();
            newvalue=newvalue%(qf.disk->metadata->range);
            for(uint64_t j=0;j<i;j++)
            {
                if(vals[j]==newvalue)
                {
                    newvalue=0;
                    break;
                }
            }
        }
        vals[i]=newvalue;


        nRepetitions[i]=(rand()%1000)+1;
    }
    double loadFactor=(double)qf.disk->metadata->noccupied_slots/(double)qf.disk->metadata->nslots;
    uint64_t insertedItems=0;
    while(insertedItems<nvals && loadFactor<0.7){
        //  printf("inserting %lu count = %lu\n",vals[insertedItems],nRepetitions[insertedItems] );
        INFO("Inserting "<< vals[insertedItems] << " Repeated "<<nRepetitions[insertedItems]);
        bufferedMQF_insert(&qf,vals[insertedItems],nRepetitions[insertedItems],false,false);
        //qf_dump(&qf);
        INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);
        count = bufferedMQF_count_key(&qf, vals[insertedItems]);
        //  CHECK(count == nRepetitions[insertedItems]);
        insertedItems++;
        loadFactor=(double)qf.disk->metadata->noccupied_slots/(double)qf.disk->metadata->xnslots;

    }
    INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);

    for(uint64_t i=0;i<insertedItems/10;i++)
    {
        bufferedMQFIterator it;
        bool res = bufferedMQF_find(&qf,&it, vals[i]);
        uint64_t key,value,count,newkey;
        it.get(&key,&value,&count);


        INFO("value = "<<vals[i]<<" Repeated " <<nRepetitions[i]);
        CHECK(count == nRepetitions[i]);


    }



}

TEST_CASE( "Inserting items( repeated 1-1000 times) in cqf(90% load factor ) and delete the memory buffer after insertion(buffered)","[buffered]" ) {
  bufferedMQF qf;
  int counter_size=4;
  srand (1);
  uint64_t qbits=15;
  uint64_t diskQbits=16;
  uint64_t num_hash_bits=qbits+8;
  uint64_t maximum_count=(1ULL<<counter_size)-1;
  INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
  bufferedMQF_init(&qf ,(1ULL<<qbits),(1ULL<<diskQbits), num_hash_bits, 0,counter_size, "tmp.ser");

  uint64_t nvals = (1ULL<<diskQbits);
  //uint64_t nvals = 3;
  uint64_t *vals;
  uint64_t *nRepetitions;
  vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
  nRepetitions= (uint64_t*)malloc(nvals*sizeof(nRepetitions[0]));
  uint64_t count;
  unordered_map<uint64_t,uint64_t> gold;

  for(uint64_t i=0;i<nvals;i++)
  {
    uint64_t newvalue=0;
    while(newvalue==0){
      newvalue=rand();
      newvalue=(newvalue<<32)|rand();
      newvalue=newvalue%(qf.memoryBuffer->metadata->range);
      for(uint64_t j=0;j<i;j++)
      {
        if(vals[j]==newvalue)
        {
          newvalue=0;
          break;
        }
      }
    }
    vals[i]=newvalue;


    nRepetitions[i]=(rand()%1000)+1;
  }
  int loadFactor=bufferedMQF_space(&qf);
  uint64_t insertedItems=0;
  while(insertedItems<nvals && loadFactor<70){
    //  printf("inserting %lu count = %lu\n",vals[insertedItems],nRepetitions[insertedItems] );
    INFO("Inserting "<< vals[insertedItems] << " Repeated "<<nRepetitions[insertedItems]);
    bufferedMQF_insert(&qf,vals[insertedItems],nRepetitions[insertedItems],false,false);
    gold[vals[insertedItems]]=nRepetitions[insertedItems];
    //qf_dump(&qf);
    INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);
    //   count = bufferedMQF_count_key(&qf, vals[insertedItems]);

    //  CHECK(count == nRepetitions[insertedItems]);
    insertedItems++;
    loadFactor=bufferedMQF_space(&qf);

  }
  INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);

//  for(uint64_t i=0;i<insertedItems;i++)
//  {
//    count = bufferedMQF_count_key(&qf, vals[i]);
//    INFO("value = "<<vals[i]<<" Repeated " <<nRepetitions[i]);
//    CHECK(count == nRepetitions[i]);
//      CHECK(gold[vals[i]] == nRepetitions[i]);
//  }
  // bufferedMQF_syncBuffer(&qf);
  bufferedMQF_deleteMemoryBuffer(&qf);
  bufferedMQFIterator* qfi= bufferedMQF_iterator(&qf, 0);
  while(!qfi->end()) {
    uint64_t key, value, count;
    qfi->get(&key, &value, &count);
    REQUIRE(count == gold[key]);
    //gold.erase(key);
    if(key==vals[0]){
      REQUIRE(count >= 5);
    }
    else{
      REQUIRE(count >= 1);
    }
    qfi->next();

  }
  //REQUIRE(gold.size()==0);


}


//
// TEST_CASE( "Counting Big counters" ){
//   bufferedMQF qf;
//   int counter_size=2;
//   srand (1);
//   uint64_t qbits=16;
//   uint64_t num_hash_bits=qbits+8;
//   uint64_t maximum_count=(1ULL<<counter_size)-1;
//   INFO("Counter size = "<<counter_size<<" max count= "<<maximum_count);
//   bufferedMQF_init(&qf, (1ULL<<qbits), num_hash_bits, 0,counter_size, true, "", 2038074761);
//
//   uint64_t nvals = (1ULL<<qbits);
//   //uint64_t nvals = 3;
//   uint64_t *vals;
//   uint64_t *nRepetitions;
//   vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
//   nRepetitions= (uint64_t*)malloc(nvals*sizeof(nRepetitions[0]));
//   uint64_t count;
//
//   for(uint64_t i=0;i<nvals;i++)
//   {
//     vals[i]=rand();
//     vals[i]=(vals[i]<<32)|rand();
//     vals[i]=vals[i]%(qf.metadata->range);
//
//     nRepetitions[i]=(rand())+1;
//   }
//   double loadFactor=(double)qf.metadata->noccupied_slots/(double)qf.metadata->nslots;
//   uint64_t insertedItems=0;
//   while(insertedItems<nvals && loadFactor<0.9){
//   //  printf("inserting %lu count = %lu\n",vals[insertedItems],nRepetitions[insertedItems] );
//     INFO("Inserting "<< vals[insertedItems] << " Repeated "<<nRepetitions[insertedItems]);
//     bufferedMQF_insert(&qf,vals[insertedItems],nRepetitions[insertedItems],false,false);
//     //bufferedMQF_dump(&qf);
//     INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);
//     count = bufferedMQF_count_key(&qf, vals[insertedItems]);
//     CHECK(count >= nRepetitions[insertedItems]);
//     insertedItems++;
//     loadFactor=(double)qf.metadata->noccupied_slots/(double)qf.metadata->nslots;
//
//   }
//   INFO("Load factor = "<<loadFactor <<" inserted items = "<<insertedItems);
//
//   for(uint64_t i=0;i<insertedItems;i++)
//   {
//     count = bufferedMQF_count_key(&qf, vals[i]);
//     INFO("value = "<<vals[i]<<" Repeated " <<nRepetitions[i]);
//     CHECK(count >= nRepetitions[i]);
//   }
//
//
//
//
// }
//
// TEST_CASE( "Removing items from cqf(90% load factor )") {
//   bufferedMQF qf;
//   int counter_size=2;
//   uint64_t qbits=16;
//   uint64_t num_hash_bits=qbits+8;
//   uint64_t maximum_count=(1ULL<<counter_size)-1;
//   bufferedMQF_init(&qf, (1ULL<<qbits), num_hash_bits, 0,counter_size, true, "", 2038074761);
//
//   uint64_t nvals = (1ULL<<qbits);
//   uint64_t *vals;
//   vals = (uint64_t*)malloc(nvals*sizeof(vals[0]));
//   for(uint64_t i=0;i<nvals;i++)
//   {
//     uint64_t newvalue=0;
//     while(newvalue==0){
//       newvalue=rand();
//       newvalue=(newvalue<<32)|rand();
//       newvalue=newvalue%(qf.metadata->range);
//       for(uint64_t j=0;j<i;j++)
//       {
//         if(vals[j]==newvalue)
//         {
//           newvalue=0;
//           break;
//         }
//       }
//     }
//     vals[i]=newvalue;
//   }
//   double loadFactor=(double)qf.metadata->noccupied_slots/(double)qf.metadata->nslots;
//   uint64_t insertedItems=0;
//   while(loadFactor<0.9){
//
//     bufferedMQF_insert(&qf,vals[insertedItems],50,false,false);
//     insertedItems++;
//     loadFactor=(double)qf.metadata->noccupied_slots/(double)qf.metadata->nslots;
//   }
//   uint64_t count;
//   for(uint64_t i=0;i<insertedItems;i++)
//   {
//     if(i%2==0){
//       count = bufferedMQF_count_key(&qf, vals[i]);
//       if(count==100){
//         printf("coubn ==100\n" );
//       }
//     bufferedMQF_remove(&qf,vals[i],50,false,false);
//     count = bufferedMQF_count_key(&qf, vals[i]);
//     CHECK(count ==0);
//     }
//   }
//   for(uint64_t i=0;i<insertedItems;i++)
//   {
//     count = bufferedMQF_count_key(&qf, vals[i]);
//     if(i%2==1){
//     CHECK(count >= 50);
//     }
//     else{
//       if(count!=0){
//         INFO("ERROR "<<vals[i]<<" Not deleted index= "<<i)
//         //printf("%lu not delete at index %lu\n", vals[i],i);
//       }
//       CHECK(count ==0);
//
//     }
//   }
//   bufferedMQFIterator qfi;
//   bufferedMQF_iterator(&qf, &qfi, 0);
//   do {
//     uint64_t key, value, count;
//     qfi_get(&qfi, &key, &value, &count);
//     count=bufferedMQF_count_key(&qf, key);
//     CHECK(count >= 50);
//   } while(!qfi_next(&qfi));
//
//
//
// }

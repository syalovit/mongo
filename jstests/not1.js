t = db.group1;
t.drop()

t.save( { a: 0 } );
t.save( { a: [ 1 , 2 , 3 ] } );
t.save( { a: [ 1 , 2 , 3 , 4 , 5 ] } );
t.save( { a: [ 2 , 3 ] } );
t.save( { a: [ 6 , 7 ] } );
t.save( { a: [ 6 , 7 , 8 , 9 , 2 , 4 , 3 ] } );
t.save( { a: [ 6 , 7 , 8 , 9 , 2 , 1 , 3 ] } );

res = t.find ( { a: { $not : { $all: [ 2, 3 ] } }   }  ).itcount();
assert.eq(res, 2, "ZZZ")

t.ensureIndex( { a : 1 } );
res = t.find ( { a: { $not : { $all: [ 2, 3 ] } }   }  ).itcount();
assert.eq(res, 2, "indexed")


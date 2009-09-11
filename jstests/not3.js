
t = db.regex3;
t.drop();

t.save( { name : "eliot" } );
t.save( { name : "emily" } );
t.save( { name : "bob" } );
t.save( { name : "aaron" } );

t.ensureIndex( { name : 1 } );

assert.eq( 2 , t.find( { name : { $not : /^e.*/ } } ).count() , "index count ne" );
// fix for indexing

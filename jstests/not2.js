
t = db.regex3;
t.drop();

t.save( { name : "eliot" } );
t.save( { name : "emily" } );
t.save( { name : "bob" } );
t.save( { name : "aaron" } );

assert.eq( 2 , t.find(  { name : { $not : /^e.*/ } }  ).count()) ;

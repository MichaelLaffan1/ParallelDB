INSERT INTO table VALUES (Comedy, Pixar, 1999)
INSERT INTO table VALUES (Drama, Universal, 2002)
INSERT INTO table VALUES (Action, Marvel, 2012)
INSERT INTO table VALUES (Drama, Paramount, 1999)
INSERT INTO table VALUES (Comedy, Universal, 2015)
SELECT * FROM table WHERE attr1=Comedy AND attr2=Pixar AND attr3=1999
SELECT * FROM table WHERE attr1=Action AND attr2=Marvel AND attr3=2012
SELECT * FROM table WHERE attr1=Drama AND attr2=Universal AND attr3=2002
INSERT INTO table VALUES (Musical, Disney, 2017)
INSERT INTO table VALUES (Western, MGM, 1962)
INSERT INTO table VALUES (Thriller, Netflix, 2021)
INSERT INTO table VALUES (Fantasy, NewLine, 2001)
INSERT INTO table VALUES (Comedy, Miramax, 1994)
INSERT INTO table VALUES (Adventure, Universal, 2015)
INSERT INTO table VALUES (Romance, Focus, 2008)
INSERT INTO table VALUES (Documentary, PBS, 2019)
INSERT INTO table VALUES (Biography, Sony, 2010)
INSERT INTO table VALUES (Crime, HBO, 2020)
INSERT INTO table VALUES (Musical, Paramount, 2016)
INSERT INTO table VALUES (Fantasy, Disney, 2017)
INSERT INTO table VALUES (Comedy, Netflix, 2021)
INSERT INTO table VALUES (Adventure, MGM, 1962)
INSERT INTO table VALUES (Biography, Netflix, 2020)
SELECT * FROM table WHERE attr1=Musical AND attr2=Disney AND attr3=2017
SELECT * FROM table WHERE attr1=Fantasy AND attr2=NewLine AND attr3=2001
SELECT * FROM table WHERE attr1=Documentary AND attr2=PBS AND attr3=2019
SELECT * FROM table WHERE attr1=Crime AND attr2=HBO AND attr3=2020
SELECT * FROM table WHERE attr1=Western AND attr2=MGM AND attr3=1962
SELECT * FROM table WHERE attr1=Musical AND attr2=Paramount AND attr3=2016
SELECT * FROM table WHERE attr1=Fantasy AND attr2=Disney AND attr3=2017
SELECT * FROM table WHERE attr1=Adventure AND attr2=Universal AND attr3=2015
SELECT * FROM table WHERE attr1=Comedy AND attr2=Miramax AND attr3=1994
SELECT * FROM table WHERE attr1=Biography AND attr2=Netflix AND attr3=2020
SELECT * FROM table WHERE attr1=Crime AND attr2=Netflix AND attr3=2020
SELECT * FROM table WHERE attr1=Western AND attr2=Disney AND attr3=1962
SELECT * FROM table WHERE attr1=Thriller AND attr2=HBO AND attr3=2021
SELECT * FROM table WHERE attr1=Romance AND attr2=Focus AND attr3=2008
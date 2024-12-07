INSERT INTO table VALUES (Drama, WarnerBros, 1999)
INSERT INTO table VALUES (Action, Universal, 2005)
INSERT INTO table VALUES (Horror, Lionsgate, 2010)
INSERT INTO table VALUES (Sci-Fi, Fox, 1997)
INSERT INTO table VALUES (Mystery, Paramount, 2018)
INSERT INTO table VALUES (Family, Disney, 2003)
INSERT INTO table VALUES (Animation, DreamWorks, 2012)
INSERT INTO table VALUES (Thriller, Sony, 2020)
INSERT INTO table VALUES (Adventure, MGM, 1985)
INSERT INTO table VALUES (Fantasy, Netflix, 2022)
INSERT INTO table VALUES (Romance, Columbia, 1995)
INSERT INTO table VALUES (Biography, HBO, 2015)
INSERT INTO table VALUES (Comedy, Amazon, 2019)
INSERT INTO table VALUES (Western, Miramax, 1975)
INSERT INTO table VALUES (Crime, Focus, 2000)
INSERT INTO table VALUES (Comedy, Sony, 2018)
INSERT INTO table VALUES (Drama, Disney, 1980)
INSERT INTO table VALUES (Romance, DreamWorks, 2000)
DELETE FROM table WHERE attr1=Biography
UPDATE SET attr1=Western, attr2=Disney WHERE attr1=Thriller
UPDATE SET attr3=2000 WHERE attr1=Biography
DELETE FROM table WHERE attr3=2000
SELECT attr1 FROM table WHERE attr1=Romance AND attr3=2000
SELECT * FROM table WHERE attr1=Western
SELECT * FROM table WHERE attr1=Biography


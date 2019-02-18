#pragma once

/*
             Narrated in game
You sit confotably in your chair and you think about your life.
You served people of your little town well and they loved you.
You were wellcomed to the big beasts of polytics and they loved 
you too - you brought all votes from your little town so they
let you have a nice office with a view and a good salary.
You are happy but unaware that real deal is just about to
begin.

Level 1 - iniciation
Your first week at the office is just passed and you just 
arrived to your workplace. To your suprise there was a strange 
tall man with short blonde hair and blue eyes sitting behind
your desk.
He introduces him self as your supperior from the office above.
He explains: "I'm so glad we finaly meet, I've heard a lot about
you lately and I timed my visit after you got comfortable... you
did get comfortable, right?" he smiled.
For the first time in your career you feel like things are
about to change. That man is here with a reason.
He continues: "You have to understand that level of comfort we
provide is not because we like you but because we need you. We
need your talents with people." He looks at you: " We need you
to work for us." He pauses.
A little confusion goes through your mind as you try to 
contemplate his words. He notices you are trying to figure him
so he immediately continues: "You know you are good, we all
know that, but your tallent is being wasted. Think about all the
lives you have not touched yet, people you still have to 
impress!"
Words "...lives you have not touched yet..." and "...have to 
impress..." ring in your mind while you wonder what lives,
impress who? He notices you wondered of for for a moment so
he tries to get your attention back before he looses you
by speaking louder, kinder, tapping you on a shoulder and 
looking you in the eyes: "You can't forget all the comforts
right? You wonder how much more can I get? What do I have
ti do, right?"
You look at him with a disagreeing look and try to say that
you really did not get here for the comforts but he quickly
continues:"I know you like it here so far so we will try
not to loose you." He laughs:"Remember just keep on doing
what you do best! Look god and act right!" He steps one
step back offers you a hand and concludes: "We will keep
in touch." You shake his hand and as he is leaving through
the doors he adds: "Just keep in mind that you are going
to get bigger and you job will require more of you." He
pauses and looks you in the eyes for the moment to make
sure you agree as he is leaving through the doors. You 
just look with wonder and he is happy with how you didn't
argue or decided to leave, after all good people skills
are all that is needed to keep the power where it
belongs. He walks out and with back turned to you and
doors almost closed he adds: "Also I had left something
on your desk. Take a look at it. It is beyond us but
you might be able to handle it."
He left the room and you take look at your desk. You
notice a small device. You decide to take a look at it.
///////////////////////GRAPHICS/////////////////////////
 - white background, black lines, isometric projection
 - closed device is displayed
 - player tap it to open (small delay then new image, no animations)
 - device has a screen and a border
 - you are presented with a wellcome message
Message:
       Wellcome!
Please wait for messages to appear.
 - couple of seconds pass and new message appears
       Hello
You have one message waiting.
 - player taps it and message appears
////////////////////////////////////////////////////////

Level 2 - New things
Message 1:
     Your services are needed. 
   We have 50 working people. We are trying to keep their
 jobs as it is main source of income for most townfolks.
 Conditions are hard, workers are unhappy mines are hardly
 profitable. What do we do:
   a) ignore requests
   b) lay off people 
   c) bring in cheap foreign labor and lay some off
   d) give them a promise of better future
   e) rise their paychecks
 /////////////////////////////////////////////////////////////
 Effects 1:
 /////////////////////////////////////////////////////////////
   a) ignore people
  irritated + 5
  angry + 1
  voters lost 50%
  number of people -10%
  ////
   Next message:
  People are getting irritated and loud. Workers strike
 seems to be innevitable. We lost a lot of voters and
 some people moved out. Small number of people is angry.
   a) ignore requests
 Effects:
   - same as above with 2xpenalties(loose all voters)
   - you loose points with both people and bosses
   - END
   b) hire informers to identify influencers
   - open new informer message
   c) bring in cheap foreign labor and lay some off
   irritated +10
   anger +3
   scared +5
   voters lost 70%
   number of people -5%
   - open new message with options to improve relations with people
   d) promise a better future
   anger -1
   voters gained 10%
   - open go back to menu 1
   e) rise their paychecks
   anger -3
   irritated -5
   happy +3
   votes gained +40%
   number of people +10%
   quality of people -20%
    ...
Bosses need high approval and lots of voters but if quality of people
drops crisis is needed to reduce number of people. Depending on number of people
crisis can be economic, illegal drugs can be introduced, crimerate can be allowed
to rise by introducing alien citizens to allow easier introduction of drugs.
 /////////////////////////////////////////////////////////////
 Level 3 keep the voters happy
 In level 3  You will need to influence some target
 groups to raise their government approval. Since this is a small
 city you can recruit people with agents who you can command
 and receive messages from. You will have two agents, one for youth 
 and one for rest of population. You will also be able to post adds
 online and have agents with fake profiles. 
 Your task is to aquire trust of leading memebers of society in their
 age group and use female agents to implicate them in to sensitive activities.
 That way you can take over police, banks and other facilities.
 //////////////////////////////////////////////////////////////////////////
 Level 4 Make money
 You have to keep the balance between voters happines and bosses happiness.
 Bosses will require from you to control the population. You will have to use 
 influence of police, banks and industry to take the money away from people using
 not just taxes, lower wages and tough laws but also silence or eliminate those
 who want to oppose the you. Using influence you have to infiltrate media, radio
 TV and news stations by buying editors and journalists.
 /////////////////////////////////////////////////////////////////////////////
 Level 5 Take the region
 You have to find a way to move the masses. Organize concerts, create stars,
 make movies, create propaganda and in general buy everybody out, mostly delivering
 expensive life style to those you want on your side.
 ////////////////////////////////////////////////////////////////////////////
 Level 6 Take the country
 You are now entitled to officially using army and police to protect your interests.
 On this level you can create anarchy and chaos or complete order in either case this
 is where direction and end game get set.
 ////////////////////////////////////////////////////////////////////////////
 Level 6 Balance
 in short you have to kill parts of population you don't like in order to create
 type of population you want to have in end game
 ////////////////////////////////////////////////////////////////////////////
 Level 7 Endgame
 A war breaks out and your country either is part of it or not. If you get attacked
 or if you decide to attack you become part of the war. Also if you own money you
 have to kill of vast ammounts of your population.
 ////////////////////////////////////////////////////////////////////////////
 At the end you are left with population and parameters and description of what
 your society is like and also a judge of your own character.
*/

namespace People{
 unsigned int people_count;

 

};























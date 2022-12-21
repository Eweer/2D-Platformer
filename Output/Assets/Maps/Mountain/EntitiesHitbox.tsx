<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.9" tiledversion="1.9.2" name="EntitiesHitbox" class="Characters" tilewidth="128" tileheight="128" tilecount="32" columns="0" objectalignment="topleft">
 <grid orientation="orthogonal" width="1" height="1"/>
 <properties>
  <property name="ColliderLayers" type="int" propertytype="ColliderLayers" value="2"/>
  <property name="FxPath" value="Assets/Audio/Fx/Player/"/>
  <property name="TexturePath" value="Assets/Animations/Player/"/>
 </properties>
 <tile id="1" class="Mage">
  <properties>
   <property name="Parameters" type="class" propertytype="Animation">
    <properties>
     <property name="AnimIteration" propertytype="AnimIteration" value="NEVER"/>
     <property name="AnimationFrame" type="int" value="1"/>
     <property name="AnimationName" propertytype="PlayerAnimation" value="Idle"/>
     <property name="AnimationSpeed" type="float" value="0.2"/>
    </properties>
   </property>
  </properties>
  <image width="128" height="128" source="../../Animations/Player/Mage/Walk/walk1.png"/>
  <objectgroup draworder="index" id="2">
   <object id="2" name="Static" class="1" x="32" y="56">
    <polygon points="0,0 -6,5 -11,17 -9,24 -4,29 -4,35 -3,42 -5,51 -2,54 24,54 29,51 30,18 28,12 25,7 19,2 12,0"/>
   </object>
  </objectgroup>
 </tile>
 <tile id="2" class="Mage">
  <properties>
   <property name="Parameters" type="class" propertytype="Animation">
    <properties>
     <property name="AnimIteration" propertytype="AnimIteration" value="LOOP_FROM_START"/>
     <property name="AnimationFrame" type="int" value="1"/>
     <property name="AnimationName" propertytype="PlayerAnimation" value="Walk"/>
     <property name="AnimationSpeed" type="float" value="0.2"/>
    </properties>
   </property>
  </properties>
  <image width="128" height="128" source="../../Animations/Player/Mage/Walk/walk2.png"/>
  <objectgroup draworder="index" id="3">
   <object id="2" name="Sensor" class="Dynamic" x="20" y="54" width="43" height="58">
    <polygon points="9,12 16,5 32,5 39,12 39,46 33,54 17,54 9,46"/>
   </object>
   <object id="4" name="Ground" class="Dynamic" x="41" y="98" width="12" height="12">
    <ellipse/>
   </object>
   <object id="6" name="BottomRight" class="Dynamic" x="51" y="93" width="12" height="12">
    <ellipse/>
   </object>
   <object id="7" name="PushRight" class="Dynamic" x="51" y="81" width="12" height="12">
    <ellipse/>
   </object>
   <object id="8" name="HoldRight" class="Dynamic" x="51" y="68" width="12" height="12">
    <ellipse/>
   </object>
   <object id="9" name="Top" class="Dynamic" x="41" y="63" width="12" height="12">
    <ellipse/>
   </object>
   <object id="10" name="HoldLeft" class="Dynamic" x="31" y="68" width="12" height="12">
    <ellipse/>
   </object>
   <object id="12" name="PushLeft" class="Dynamic" x="31" y="81" width="12" height="12">
    <ellipse/>
   </object>
   <object id="13" name="BottomLeft" class="Dynamic" x="31" y="93" width="12" height="12">
    <ellipse/>
   </object>
  </objectgroup>
  <animation>
   <frame tileid="2" duration="170"/>
   <frame tileid="3" duration="170"/>
   <frame tileid="4" duration="170"/>
   <frame tileid="5" duration="170"/>
   <frame tileid="6" duration="170"/>
  </animation>
 </tile>
 <tile id="3" class="Mage">
  <properties>
   <property name="Parameters" type="class" propertytype="Animation">
    <properties>
     <property name="AnimIteration" propertytype="AnimIteration" value="LOOP_FROM_START"/>
     <property name="AnimationFrame" type="int" value="2"/>
     <property name="AnimationName" propertytype="PlayerAnimation" value="Walk"/>
     <property name="AnimationSpeed" type="float" value="0.2"/>
    </properties>
   </property>
  </properties>
  <image width="128" height="128" source="../../Animations/Player/Mage/Walk/walk3.png"/>
 </tile>
 <tile id="4" class="Mage">
  <properties>
   <property name="Parameters" type="class" propertytype="Animation">
    <properties>
     <property name="AnimIteration" propertytype="AnimIteration" value="LOOP_FROM_START"/>
     <property name="AnimationFrame" type="int" value="3"/>
     <property name="AnimationName" propertytype="PlayerAnimation" value="Walk"/>
     <property name="AnimationSpeed" type="float" value="0.2"/>
    </properties>
   </property>
  </properties>
  <image width="128" height="128" source="../../Animations/Player/Mage/Walk/walk4.png"/>
 </tile>
 <tile id="5" class="Mage">
  <properties>
   <property name="Parameters" type="class" propertytype="Animation">
    <properties>
     <property name="AnimIteration" propertytype="AnimIteration" value="LOOP_FROM_START"/>
     <property name="AnimationFrame" type="int" value="4"/>
     <property name="AnimationName" propertytype="PlayerAnimation" value="Walk"/>
     <property name="AnimationSpeed" type="float" value="0.2"/>
    </properties>
   </property>
  </properties>
  <image width="128" height="128" source="../../Animations/Player/Mage/Walk/walk5.png"/>
 </tile>
 <tile id="6" class="Mage">
  <properties>
   <property name="Parameters" type="class" propertytype="Animation">
    <properties>
     <property name="AnimIteration" propertytype="AnimIteration" value="LOOP_FROM_START"/>
     <property name="AnimationFrame" type="int" value="5"/>
     <property name="AnimationName" propertytype="PlayerAnimation" value="Walk"/>
     <property name="AnimationSpeed" type="float" value="0.2"/>
    </properties>
   </property>
  </properties>
  <image width="128" height="128" source="../../Animations/Player/Mage/Walk/walk6.png"/>
 </tile>
 <tile id="7" class="Dwarf">
  <properties>
   <property name="Parameters" type="class" propertytype="Animation">
    <properties>
     <property name="AnimIteration" propertytype="AnimIteration" value="LOOP_FROM_START"/>
     <property name="AnimationFrame" type="int" value="1"/>
     <property name="AnimationName" propertytype="PlayerAnimation" value="Walk"/>
     <property name="AnimationSpeed" type="float" value="0.2"/>
    </properties>
   </property>
  </properties>
  <image width="128" height="128" source="../../Animations/Enemies/Mountain/Dwarf/Walk/walk1.png"/>
  <objectgroup draworder="index" id="2">
   <object id="2" name="Ground" class="Dynamic" x="41" y="84" width="7" height="7">
    <ellipse/>
   </object>
   <object id="9" name="CharacterSensor" class="Dynamic" x="21" y="44" width="43" height="46">
    <polygon points="0,0 21,0 43,0 43,23 43,46 22,46 0,46 0,21"/>
   </object>
  </objectgroup>
  <animation>
   <frame tileid="7" duration="170"/>
   <frame tileid="8" duration="170"/>
   <frame tileid="9" duration="170"/>
   <frame tileid="10" duration="170"/>
   <frame tileid="11" duration="170"/>
   <frame tileid="12" duration="170"/>
  </animation>
 </tile>
 <tile id="8">
  <image width="128" height="128" source="../../Animations/Enemies/Mountain/Dwarf/Walk/walk2.png"/>
 </tile>
 <tile id="9">
  <image width="128" height="128" source="../../Animations/Enemies/Mountain/Dwarf/Walk/walk3.png"/>
 </tile>
 <tile id="10">
  <image width="128" height="128" source="../../Animations/Enemies/Mountain/Dwarf/Walk/walk4.png"/>
 </tile>
 <tile id="11">
  <image width="128" height="128" source="../../Animations/Enemies/Mountain/Dwarf/Walk/walk5.png"/>
 </tile>
 <tile id="12">
  <image width="128" height="128" source="../../Animations/Enemies/Mountain/Dwarf/Walk/walk6.png"/>
 </tile>
 <tile id="13">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire1.png"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="1" y="21" width="30" height="8">
    <polygon points="0,3 8,0 26,0 28,3 28,5 26,8 8,8 0,5"/>
   </object>
   <object id="2" x="0" y="24" width="2" height="2">
    <ellipse/>
   </object>
  </objectgroup>
  <animation>
   <frame tileid="13" duration="170"/>
   <frame tileid="14" duration="170"/>
   <frame tileid="15" duration="170"/>
  </animation>
 </tile>
 <tile id="14">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire2.png"/>
 </tile>
 <tile id="15">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire3.png"/>
 </tile>
 <tile id="16">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire4.png"/>
 </tile>
 <tile id="17">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire5.png"/>
 </tile>
 <tile id="18">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire6.png"/>
 </tile>
 <tile id="19">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire7.png"/>
 </tile>
 <tile id="20">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire8.png"/>
 </tile>
 <tile id="21">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire/fire9.png"/>
 </tile>
 <tile id="22">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra1.png"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="20" height="32">
    <polygon points="6,0 19,10 19,22 10,32 0,32 8,23 8,9 0,0"/>
   </object>
  </objectgroup>
  <animation>
   <frame tileid="22" duration="170"/>
   <frame tileid="23" duration="170"/>
   <frame tileid="24" duration="170"/>
  </animation>
 </tile>
 <tile id="23">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra2.png"/>
 </tile>
 <tile id="24">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra3.png"/>
 </tile>
 <tile id="25">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra4.png"/>
 </tile>
 <tile id="26">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra5.png"/>
 </tile>
 <tile id="27">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra6.png"/>
 </tile>
 <tile id="28">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra7.png"/>
 </tile>
 <tile id="29">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra8.png"/>
 </tile>
 <tile id="30">
  <image width="32" height="32" source="../../Animations/Player/Mage/Fire_Extra/fire_extra9.png"/>
 </tile>
 <tile id="31">
  <image width="128" height="128" source="../../Animations/Player/Knight/Idle/idle1.png"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="32" y="70" width="12" height="31"/>
   <object id="2" x="44" y="70" width="12" height="31"/>
   <object id="4" x="32" y="88" width="24" height="22">
    <ellipse/>
   </object>
   <object id="5" x="44" y="60">
    <polyline points="0,0 0,50"/>
   </object>
   <object id="6" x="32" y="60" width="24" height="22">
    <ellipse/>
   </object>
  </objectgroup>
 </tile>
 <tile id="32">
  <image format="png">
   <data encoding="base64"></data>
  </image>
  <objectgroup draworder="index" id="2">
   <object id="1" x="28" y="35" width="72" height="58">
    <polygon points="-1,39 -1,13 15,-1 42,4 73,46 48,46 30,58 10,58"/>
   </object>
  </objectgroup>
 </tile>
</tileset>

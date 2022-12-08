<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.9" tiledversion="1.9.2" name="EntitiesHitbox" class="Characters" tilewidth="128" tileheight="128" tilecount="6" columns="0" objectalignment="topleft">
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
    <polygon points="5,13 14,3 32,4 43,15 42,33 37,40 38,50 34,53 12,53 9,50"/>
   </object>
   <object id="4" name="Ground" class="Dynamic" x="40" y="103" width="7" height="7">
    <ellipse/>
   </object>
   <object id="6" name="BottomRight" class="Dynamic" x="51" y="96" width="7" height="7">
    <ellipse/>
   </object>
   <object id="7" name="Front" class="Dynamic" x="56" y="75" width="7" height="7">
    <ellipse/>
   </object>
   <object id="8" name="TopRight" class="Dynamic" x="52" y="62" width="7" height="7">
    <ellipse/>
   </object>
   <object id="9" name="Top" class="Dynamic" x="39" y="57" width="7" height="7">
    <ellipse/>
   </object>
   <object id="10" name="TopLeft" class="Dynamic" x="25" y="65" width="7" height="7">
    <ellipse/>
   </object>
   <object id="11" name="BottomLeft" class="Dynamic" x="28" y="91" width="7" height="7">
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
</tileset>

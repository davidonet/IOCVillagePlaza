class ActivitiesConnection extends Sequence
{
  Ribbon tagRibbon; 
  int yShift;
  Sequence nextSeq;

  ActivitiesConnection(String aLang)
  { 
    super("/sdcard/Storages/home.png");
    tagRibbon = new Ribbon(14);

    if ("en" == aLang) {
      tagRibbon.addSection(280, 0, 320, 0);
    }
    if ("fr" == aLang) {
      tagRibbon.addSection(400, 0, 440, 0);
    }
    if ("es" == aLang) {
      tagRibbon.addSection(520, 0, 560, 0);
    }
    if ("ru" == aLang) {
      tagRibbon.addSection(640, 0, 680, 0);
    }
    if ("ar" == aLang) {
      tagRibbon.addSection(760, 0, 800, 0);
    }
    if ("cn" == aLang) {
      tagRibbon.addSection(880, 0, 920, 0);
    }
    tagRibbon.addSection(268, 80, 300, 100);
    tagRibbon.addSection(320, 60, 360, 100);
    tagRibbon.addSection(240, 280, 280, 260);
    tagRibbon.addSection(160, 280, 200, 220);
    tagRibbon.addSection(220, 500, 260, 520);
    tagRibbon.addSection(300, 460, 340, 500);
    tagRibbon.addSection(220, 740, 240, 760);
    tagRibbon.addSection(280, 760, 300, 800);
    tagRibbon.addSection(168, 848, 160, 880);
    tagRibbon.addSection(200, 884, 192, 920);
    tagRibbon.addSection(72, 904, 56, 936);
    tagRibbon.addSection(96, 948, 88, 976);
    tagRibbon.addSection(0, 960, 0, 996);
    nextSeq = new ActivitiesPage(aLang);
    setup();
    theHomePage.activitiesPage.put(aLang, this);
  }
  void setup()
  {
    tagRibbon.resetAnimation();
    yShift=800;
  }
  Sequence draw()
  {
    if (-799<yShift)
    {
      if (0<yShift)
        yShift-=(900+yShift)/160;
      else
        yShift-=-(-900+yShift)/144;
    }
    translate(0, yShift);
    if (0<yShift)
      image(getImg(), 0, -800);
    if (!tagRibbon.doAnimateRecover())
    {
      nextSeq.setup();
      return nextSeq;
    }
    return this;
  }
}

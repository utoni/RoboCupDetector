#include "RC_BlobDetector.hpp"


#define measureTicks(var) { var = cv::getTickCount(); }
#define calcDiffTime(start, end, dest) { float tmp = dest; dest = (end - start) / cv::getTickFrequency(); dest = (dest + tmp)/2; }


cv::Mat rc::BlobDetector::process(cv::Mat& image, enum rc::roboColor rc, rc::processed_image& pi, rc::time_consumption& tc) {
  int64_t start, end;
  cv::Mat imgHSV, imgThresh;

  measureTicks(start);
  /* Farbe filtern */
  imgThresh = this->filterColor(image, rc);
  measureTicks(end);
  calcDiffTime(start, end, tc.avg_color);
  measureTicks(start);
  /* Bild unscharf gestalten (Bildfehler/Farbrauschen korrigieren) */
  /* Hinweis: Median Blur ist sehr langsam */
  //cv::medianBlur(imgThresh, imgThresh, 13);
  cv::blur(imgThresh, imgThresh, cv::Size(4,4), cv::Point(-1,-1), cv::BORDER_REPLICATE);
  measureTicks(end);
  calcDiffTime(start, end, tc.avg_blur);

  /* Konturen suchen und im Vector `contours` speichern */
  std::vector<std::vector<cv::Point>> contours;
  cv::Mat imgContour = imgThresh.clone();
  measureTicks(start);
  cv::findContours(imgContour, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
  measureTicks(end);
  calcDiffTime(start, end, tc.avg_contours);

  /* nach größter Kontur suchen */
  double largest_area = 0.0;
  int largest_contour_index = -1;
  for (size_t i = 0; i < contours.size(); i++) {
    /* berechne Größe der Fläche (Green's Integral) */
    auto a = contourArea(contours[i], false);
    if (a > largest_area) {
      largest_area = a;
      largest_contour_index = i;
    }
  }

  /* Konturen im original Bild nachzeichnen, dabei ein Rechteck als Form der Kontur annehmen */
  if (largest_contour_index >= 0 && largest_area > 2000) {
    auto idx = largest_contour_index;

    measureTicks(start);

    /* Konturen nachzeichnen */
    cv::drawContours(image, contours, idx, cv::Scalar(255,0,0));
    /* minimales Rechteck in die Kontur einpassen */
    cv::RotatedRect rotRect = cv::minAreaRect(contours[idx]);
    /* vier Eckpunkte des berechneten Rechteckes speichern */
    cv::Point2f rect_points[4]; 
    rotRect.points(rect_points);
    /* Winkel speichern */
    float angle = rotRect.angle;
    /* Mittelpunkt des Rechteckes */
    cv::Point2f center = rotRect.center;

    /* Konturen nachzeichen, Rechteck zeichnen, Mittelpunkt zeichnen, Text schreiben */
    for (size_t jdx = 0; jdx < 4; ++jdx)
      cv::line(image, rect_points[jdx], rect_points[(jdx+1)%4], cv::Scalar(0,255,0));
    std::stringstream ss; ss << angle << "/" << largest_area;
    cv::circle(image, center, 5, cv::Scalar(0,255,0));
    cv::putText(image, ss.str(), center + cv::Point2f(-25,25), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(255,0,255));

    measureTicks(end);
    calcDiffTime(start, end, tc.avg_draw);

    /* Ausgabedatenstruktur beschreiben für später Verwendung (z.B.: Transport über SPI/I²C Schnittstelle) */
    for (size_t idx = 0; idx < 4; ++idx)
      pi.coords[idx] = rect_points[idx];
    pi.angle = angle;
    pi.distance = largest_area;
  }

  /* zum Schluß das Bild mit Farbfilter zurückgeben */
  return imgThresh;
}

// Linki do innych stron otwieraj w nowej karcie.
function linkiWNowejKarcie() {
  document.querySelectorAll("a[href^='http']").forEach(function (a) {
    if (a.host !== window.location.host) {
      a.target = "_blank";
      a.rel = "noopener noreferrer";
    }
  });
}

if (typeof document$ !== "undefined") {
  document$.subscribe(linkiWNowejKarcie);
} else {
  document.addEventListener("DOMContentLoaded", linkiWNowejKarcie);
}

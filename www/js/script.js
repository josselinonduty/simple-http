document.addEventListener("DOMContentLoaded", function () {
  var p = document.createElement("p");
  p.className = "generated-at";
  p.textContent = `Generated at ${new Date().toISOString()}`;
  document.querySelector("#container").appendChild(p);
});

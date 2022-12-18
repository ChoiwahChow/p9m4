

LoadPackage("magmaut");;

Read("models.g");;

non_iso := [];;

for x in m do
  g := 1; 
  for y in non_iso do
    if not IsomorphismAlgebras(x, y) = fail then
      g := 0;
      break;
    fi;
  od;
  if g = 1 then Add(non_iso, x); fi;
od;

Print(Length(non_iso));

#! /bin/sh

[ "$1" ] && PREFIX="$1"

if [ ! "${PREFIX}" ]; then
  echo "Error: PREFIX not set!"
  exit 1
fi

export PREFIX

[ -d "${PREFIX}/docs}" ] || mkdir -p "${PREFIX}/docs"

if [ -f "${PREFIX}/etc/max.ctl" ]; then
  echo "This is not a fresh install -- not copying install tree.."
else
  echo "Copying install tree to ${PREFIX}.."
  cp -rp install_tree/* "${PREFIX}"

  if [ "${PREFIX}" != "/var/max" ]; then
    echo "Modifying configuration files to reflect PREFIX=${PREFIX}.."
    for file in etc/max.ctl etc/areas.bbs etc/compress.cfg etc/squish.cfg
    do
      echo " - ${file}"
      cat "${PREFIX}/${file}" | sed "s;/var/max;${PREFIX};g" > "${PREFIX}/${file}.tmp"
      mv -f "${PREFIX}/${file}.tmp" "${PREFIX}/${file}"
    done
  fi
fi

if [ -f "${PREFIX}/bin/runbbs.sh" ]; then
  echo "This is not a fresh install -- not copying runbbs.sh.."
else
  cp scripts/runbbs.sh "${PREFIX}/bin/runbbs.sh"
fi

cp docs/max_mast.txt "${PREFIX}/docs"

exit 0

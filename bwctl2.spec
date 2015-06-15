%{!?python_sitelib: %define python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib()")}

Name: bwctl2
Version: 2.0
Release: 0.0.a3%{?dist}
Summary: Network measurement scheduler
Group: *Development/Libraries*
URL: http://software.internet2.edu/bwctl
License: Apache License v2.0

Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch: noarch

BuildRequires: python-setuptools

# Make the 'bwctl' a metapackage that installs the client and server
Requires: bwctl2-client
Requires: bwctl2-server

%description
BWCTL is a command line client application and a scheduling and policy daemon
that wraps various network measurement tools including iperf, iperf3, owamp
ping and traceroute.

%package client
Summary: bwctl client
Group: Applications/Network
Requires:   iperf, iperf3 >= 3.0.11, bwctl2-shared
%description client
bwctl command line tool for scheduling bandwidth measurements with a bwctl
server.

%package server
Summary: bwctl server
Group: Applications/Network
Requires: chkconfig, initscripts, shadow-utils, coreutils
Requires:   iperf, iperf3 >= 3.0.11, bwctl2-shared
%description server
bwctl server

%package shared
Summary: bwctl shared components
Group: Applications/Network

Requires: python-psutil
Requires: python-configobj
Requires: py-radix
Requires: python-cherrypy
Requires: python-routes
Requires: python-setuptools
Requires: python-simplejson
Requires: python-netifaces
Requires: uuid

%description shared
Shared components used by the bwctl server and client RPMs

%pre
/usr/sbin/groupadd bwctl2 2> /dev/null || :
/usr/sbin/useradd -g bwctl2 -r -s /sbin/nologin -c "BWCTL2 User" -d /tmp bwctl2 2> /dev/null || :

%prep
%setup -q

%build
%{__python} setup.py build

%install
%{__python} setup.py install --skip-build --root %{buildroot}
install -D -m755 scripts/bwctld.init %{buildroot}/%{_initrddir}/bwctld2

%files

%files client
%defattr(-,root,root)
%{_bindir}/bwctl2
%{_bindir}/bwping2
%{_bindir}/bwtraceroute2

%files shared
%defattr(-,root,root)
%doc
%{python_sitelib}/bwctl/*
%{python_sitelib}/bwctl*.egg-info/

%files server
%defattr(-,root,root)
%doc
%{_bindir}/bwctld2
%config(noreplace) %{_sysconfdir}/bwctld2/bwctld.conf
#%config(noreplace) %{_sysconfdir}/bwctld2/bwctld.limits
%config(noreplace) %{_initrddir}/bwctld2

%post server
if [ $1 = 0 ]; then
    /sbin/chkconfig --add bwctld
fi
touch /var/log/perfsonar/bwctld.log
chown bwctl2:bwctl2 /var/log/perfsonar/bwctld.log

%preun server
if [ $1 = 0 ]; then
    /sbin/service bwctld stop >/dev/null 2>&1
    /sbin/chkconfig --del bwctld
fi

%changelog
* Wed Mar 11 2015 Aaron Brown <aaron@internet2.edu> - 2.0a1
Initial version

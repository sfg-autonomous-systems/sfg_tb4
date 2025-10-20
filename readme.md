# TB4

## Create 3 Setup

Below instructions only work on our modified TB4s that include NVIDIA Jetson hardware running this repository.

1. Ensure the Create 3 platform is up-to-date by following the initial setup [here](https://edu.irobot.com/create3-setup).
2. Configure the following settings via the Create 3's web-interface. Note that any other setting should remain at its respective default value.

    <table>
      <thead>
        <tr>
          <th>Setting</th>
          <th>Value</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td>Application/Configuration/ROS 2 Domain ID</td>
          <td>Same as configured on the NVIDIA Jetson running this repository.</td>
        </tr>
        <tr>
          <td>Application/Configuration/ROS 2 Namespace</td>
          <td><code>/local/&lt;agent&gt;/create3</code></td>
        </tr>
        <tr>
          <td>Application/Configuration/RMW_IMPLEMENTATION</td>
          <td><code>rmw_fastrtps_cpp</code></td>
        </tr>
        <tr>
          <td>Beta Features/Set Wired Subnet/New Robot IP</td>
          <td><code>192.168.55.2</code></td>
        </tr>
          <tr>
          <td>Beta Features/Edit ntp.conf</td>
          <td><code>server 192.168.50.1 prefer iburst minpoll 4 maxpoll 6</code></td>
        </tr>
        <tr>
          <td>Beta Features/RMW Profile Override</td>
          <td>
            <pre><code>&lt;?xml version="1.0" encoding="UTF-8" ?&gt;
    &lt;profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles"&gt;
        &lt;participant profile_name="unicast_discovery" is_default_profile="true"&gt;
            &lt;rtps&gt;
                &lt;builtin&gt;
                    &lt;metatrafficUnicastLocatorList&gt;
                        &lt;locator/&gt;
                    &lt;/metatrafficUnicastLocatorList&gt;
                    &lt;initialPeersList&gt;
                        &lt;locator&gt;
                            &lt;udpv4&gt;
                                &lt;address&gt;127.0.0.1&lt;/address&gt;
                                &lt;address&gt;192.168.55.3&lt;/address&gt;
                            &lt;/udpv4&gt;
                        &lt;/locator&gt;
                    &lt;/initialPeersList&gt;
                &lt;/builtin&gt;
            &lt;/rtps&gt;
        &lt;/participant&gt;
    &lt;/profiles&gt;</code></pre>
          </td>
        </tr>
      </tbody>
    </table>
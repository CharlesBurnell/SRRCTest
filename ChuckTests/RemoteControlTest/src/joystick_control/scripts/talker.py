#!/usr/bin/env python
# Software License Agreement (BSD License)
#
# Copyright (c) 2008, Willow Garage, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Revision $Id$

## Simple talker demo that published std_msgs/Strings messages
## to the 'chatter' topic

import pygame
import rospy
from std_msgs.msg import String

def talker():
    pub = rospy.Publisher('chatter', String, queue_size=10)
    pub0 = rospy.Publisher('0_axis', String, queue_size = 1)
    pub1 = rospy.Publisher('1_axis', String, queue_size = 1)
    pub2 = rospy.Publisher('2_axis', String, queue_size = 1)
    pub3 = rospy.Publisher('3_axis', String, queue_size = 1)
    pub_list = [pub0, pub1, pub2, pub3]
    rospy.init_node('talker', anonymous=True)
    rate = rospy.Rate(10) # 10hz
    pygame.init()
    pygame.joystick.init()
    joystick_count = pygame.joystick.get_count()
    while not rospy.is_shutdown():
        for i in range(joystick_count):
            pygame.event.get()
            joystick = pygame.joystick.Joystick(i)
            joystick.init()
        
        
            # Get the name from the OS for the controller/joystick
            
            # Usually axis run in pairs, up/down for one, and left/right for
            # the other.
            axes = joystick.get_numaxes()
             
            for i in range( axes ):
                axis = joystick.get_axis( i )
                rospy.loginfo(rospy.get_caller_id() + "Axis {} value: {:>6.3f}".format(i, axis))
                print(type(axis));
                pub_list[i].publish(str(axis));
                #textPrint.Print(screen, "Axis {} value: {:>6.3f}".format(i, axis) )
        rate.sleep()
'''     Commented out cause we don't need it yet
            buttons = joystick.get_numbuttons()

            for i in range( buttons ):
                button = joystick.get_button( i )
                #textPrint.Print(screen, "Button {:>2} value: {}".format(i,button) )
                
            # Hat switch. All or nothing for direction, not like joysticks.
            # Value comes back in an array.
            hats = joystick.get_numhats()

            for i in range( hats ):
                hat = joystick.get_hat( i )
                #textPrint.Print(screen, "Hat {} value: {}".format(i, str(hat)) )
'''

if __name__ == '__main__':
    try:
        talker()
    except rospy.ROSInterruptException:
        pass

// VZ Image Compare (C) 2020 Stephane Charette <stephanecharette@gmail.com>
// MIT license applies.  See "license.txt" for details.
// $Id: VzImgCmp.cpp 2964 2020-05-04 03:14:36Z stephane $

#include <VzImgCmp.hpp>

vz::ImgCmp::ImgCmp()
{
	reset();

	return;
}


vz::ImgCmp::ImgCmp(cv::Mat master)
{
	reset();

	set_master_image(master);

	return;
}


vz::ImgCmp::~ImgCmp()
{
	return;
}


vz::ImgCmp & vz::ImgCmp::reset()
{
	annotation_colour = cv::Scalar(0, 0, 255); // pure red

	master_original			.clear();
	master_resized			.clear();
	candy_original			.clear();
	candy_resized			.clear();
	differences_contours	.clear();
	differences				= cv::Mat();
	differences_threshold	= cv::Mat();
	annotated_candy 		= cv::Mat();
	annotation_thickness	= 2;
	dilate_and_erode		= 3;
	resized_image_scale		= 0.25;
	min_contour_area		= 5.0;
	flags					= Flags::kNone;
	set_flag(Flags::kAnnotateAddGreenBorder);
	set_flag(Flags::kAnnotateAddRedBorder);
	set_flag(Flags::kDrawContour);

	verify_flags();

	return *this;
}


std::string vz::ImgCmp::version() const
{
	return VZIC_VERSION;
}


vz::ImgCmp & vz::ImgCmp::set_flag(const Flags f, const bool verify)
{
	flags = static_cast<Flags>(flags | f);

	// if we've set one of these flags, then we need to clear the corresponding one
	std::map<Flags, Flags> m =
	{
		{Flags::kDiffResized		, Flags::kDiffOriginalSize	},
		{Flags::kDiffOriginalSize	, Flags::kDiffResized		},

		{Flags::kDiffGreyscale		, Flags::kDiffColour		},
		{Flags::kDiffColour			, Flags::kDiffGreyscale		},

		{Flags::kThresholdTriangle	, Flags::kThresholdOtsu		},
		{Flags::kThresholdOtsu		, Flags::kThresholdTriangle	},

		{Flags::kAnnotateOverGrey	, Flags::kAnnotateOverColour},
		{Flags::kAnnotateOverColour	, Flags::kAnnotateOverGrey	},
	};
	if (m.count(f) == 1)
	{
		flags = static_cast<Flags>(flags & ~m.at(f));
	}

	if (verify)
	{
		verify_flags();
	}

	return *this;
}


vz::ImgCmp & vz::ImgCmp::clear_flag(const Flags f, const bool verify)
{
	flags = static_cast<Flags>(flags & ~f);

	// if we've cleared one of these flags, then we need to set the corresponding one
	std::map<Flags, Flags> m =
	{
		{Flags::kDiffResized		, Flags::kDiffOriginalSize	},
		{Flags::kDiffOriginalSize	, Flags::kDiffResized		},

		{Flags::kDiffGreyscale		, Flags::kDiffColour		},
		{Flags::kDiffColour			, Flags::kDiffGreyscale		},

		{Flags::kThresholdTriangle	, Flags::kThresholdOtsu		},
		{Flags::kThresholdOtsu		, Flags::kThresholdTriangle	},

		{Flags::kAnnotateOverGrey	, Flags::kAnnotateOverColour},
		{Flags::kAnnotateOverColour	, Flags::kAnnotateOverGrey	},
	};
	if (m.count(f) == 1)
	{
		flags = static_cast<Flags>(flags | m.at(f));
	}

	if (verify)
	{
		verify_flags();
	}

	return *this;
}


vz::ImgCmp & vz::ImgCmp::verify_flags()
{
	// exactly 1 option per group can be enabled
	const std::vector<std::vector<Flags>> groupings =
	{
		{Flags::kDiffResized		, Flags::kDiffOriginalSize	},
		{Flags::kDiffGreyscale		, Flags::kDiffColour		},
		{Flags::kThresholdTriangle	, Flags::kThresholdOtsu		},
		{Flags::kAnnotateOverGrey	, Flags::kAnnotateOverColour},
	};

	for (const auto group : groupings)
	{
		// check to see how many flags from this group have been set
		size_t number_of_flags_set_in_this_group = 0;
		for (const auto f : group)
		{
			if (flags & f)
			{
				number_of_flags_set_in_this_group ++;
			}
		}

		// we know *exactly* 1 flag from this group should be set
		if (number_of_flags_set_in_this_group == 0)
		{
			// since none are set, default to the first one
			set_flag(group[0]);
		}
		else if (number_of_flags_set_in_this_group > 1)
		{
			// too many flags have been set, so remove all of them except for the first one
			bool found_first_flag = false;
			for (const auto f : group)
			{
				if ((flags & f) && found_first_flag == false)
				{
					found_first_flag = true;
				}
				else
				{
					clear_flag(f);
				}
			}
		}
	}

	// check several other settings to ensure we're using sane values

	resized_image_scale	= std::clamp(resized_image_scale, 0.1	, 4.0		);
	dilate_and_erode	= std::clamp(dilate_and_erode	, 0		, 50		);
	min_contour_area	= std::clamp(min_contour_area	, 0.0	, 999999.0	);

	return *this;
}


vz::VStr vz::ImgCmp::debug_save()
{
	VStr v;

	std::stringstream ss;
	ss	<< std::time(nullptr) << "-"
		<< (flags & Flags::kDiffResized			? "resized_" + std::to_string((int)std::round(100.0 * resized_image_scale)) + "_pct-" : "")
		<< (flags & Flags::kDiffOriginalSize	? "full_size-"			: "")
		<< differences_contours.size()			<< "_contours-"
		<< (int)std::round(min_contour_area)	<< "_minarea-"
		<< dilate_and_erode						<< "_dilate_erode-"
		<< (flags & Flags::kDiffColour			? "colour_diff-"		: "")
		<< (flags & Flags::kDiffGreyscale		? "greyscale_diff-"		: "")
		<< (flags & Flags::kThresholdTriangle	? "triangle_threshold-"	: "")
		<< (flags & Flags::kThresholdOtsu		? "otsu_threshold-"		: "")
		<< (flags & Flags::kDrawContour			? "draw_contours-"		: "")
		<< (flags & Flags::kDrawRectangle		? "draw_rectangles-"	: "")
		;
	const std::string prefix = ss.str();

	std::string fn;
	fn = prefix + "1-master_original.png"	; v.push_back(fn); cv::imwrite(fn, master_original	.get()	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "2-master_resized.png"	; v.push_back(fn); cv::imwrite(fn, master_resized	.get()	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "3-candy_original.png"	; v.push_back(fn); cv::imwrite(fn, candy_original	.get()	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "4-candy_resized.png"		; v.push_back(fn); cv::imwrite(fn, candy_resized	.get()	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "5-differences.png"		; v.push_back(fn); cv::imwrite(fn, differences				, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "6-diff_threshold.png"	; v.push_back(fn); cv::imwrite(fn, differences_threshold	, {cv::IMWRITE_PNG_COMPRESSION, 9});
	fn = prefix + "7-annotated.png"			; v.push_back(fn); cv::imwrite(fn, annotated_candy			, {cv::IMWRITE_PNG_COMPRESSION, 9});

	return v;
}


vz::ImgCmp & vz::ImgCmp::set_master_image(cv::Mat img)
{
	verify_flags();
	master_original			.clear();
	master_resized			.clear();
	candy_original			.clear();
	candy_resized			.clear();
	differences_contours	.clear();
	differences				= cv::Mat();
	differences_threshold	= cv::Mat();
	annotated_candy			= cv::Mat();

	if (img.empty()			||
		img.channels() != 3	||
		img.cols < 10		||
		img.rows < 10		)
	{
		/// @throws std::invalid_argument if the new master image seems to be invalid
		throw std::invalid_argument("master image is invalid");
	}

	master_original.set_original(img);

	cv::Mat tmp = vz::resize(img, resized_image_scale);
	master_resized.set_original(tmp);

	return *this;
}


size_t vz::ImgCmp::compare(cv::Mat img)
{
	verify_flags();
	candy_original			.clear();
	candy_resized			.clear();
	differences_contours	.clear();
	differences				= cv::Mat();
	differences_threshold	= cv::Mat();
	annotated_candy			= cv::Mat();

	if (master_original.empty())
	{
		/// @throw std::logic_error if the master image has not yet been defined
		throw std::logic_error("cannot compare image when the master image has not yet been defined");
	}

	if (img.empty()			||
		img.channels() != 3	||
		img.cols < 10		||
		img.rows < 10		)
	{
		/// @throws std::invalid_argument if the new image seems to be invalid
		throw std::invalid_argument("image is invalid");
	}

	candy_original = img;
	candy_resized = vz::resize(img, resized_image_scale);

	diff_images();
	get_contours();

	return differences_contours.size();
}


vz::ImgCmp & vz::ImgCmp::diff_images()
{
	cv::Mat img1;
	cv::Mat img2;
	auto type = vz::Imagination::EMatType::kOriginalImage;
	if (flags & Flags::kDiffGreyscale)
	{
		type = vz::Imagination::EMatType::kO2BlurGreyscale;
	}
	if (flags & Flags::kDiffResized)
	{
		img1 = master_resized	.get(type);
		img2 = candy_resized	.get(type);
	}
	else
	{
		img1 = master_original	.get(type);
		img2 = candy_original	.get(type);
	}

	cv::absdiff(img1, img2, differences);
	vz::Imagination diff(differences);

	type = vz::Imagination::EMatType::kThresholdTriangle;
	if (flags & Flags::kThresholdOtsu)
	{
		type = vz::Imagination::EMatType::kThresholdOtsu;
	}
	cv::Mat threshold = diff.get(type);
	if (dilate_and_erode > 0)
	{
		cv::Mat mat1 = vz::dilate(threshold, dilate_and_erode);
		cv::Mat mat2 = vz::erode(mat1, dilate_and_erode);
		threshold = mat2;
	}
	differences_threshold = threshold;

	return *this;
}


vz::ImgCmp & vz::ImgCmp::get_contours()
{
	differences_contours.clear();

	for (const auto & contour : vz::find_contours(differences_threshold))
	{
		const double area = cv::contourArea(contour);
		if (area >= min_contour_area)
		{
			differences_contours.push_back(contour);
//			std::cout << "-> difference #" << differences_contours.size() << " has an area measuring " << area << " square pixels" << std::endl;
		}
		else
		{
//			std::cout << "-> skipping difference because the area only measures " << area << std::endl;
		}
	}

	return *this;
}


cv::Mat vz::ImgCmp::annotate()
{
	if (flags & Flags::kDiffResized)
	{
		annotated_candy = candy_resized.get().clone();
	}
	else
	{
		annotated_candy = candy_original.get().clone();
	}

	if (flags & Flags::kAnnotateOverGrey)
	{
		cv::Mat mat1;
		cv::Mat mat2;
		cv::cvtColor(annotated_candy, mat1, cv::COLOR_BGR2GRAY);
		cv::cvtColor(mat1, mat2, cv::COLOR_GRAY2BGR);

		// copy only the masked differences over from the colour image into mat2
		annotated_candy.copyTo(mat2, differences_threshold);
		annotated_candy = mat2;
	}

	if (differences_contours.empty() && flags & Flags::kAnnotateAddGreenBorder)
	{
		cv::Rect r(0, 0, annotated_candy.cols, annotated_candy.rows);
		cv::rectangle(annotated_candy, r, cv::Scalar(0, 255, 0), annotation_thickness, cv::LINE_AA);
	}
	else if (differences_contours.empty() == false && flags & Flags::kAnnotateAddRedBorder)
	{
		cv::Rect r(0, 0, annotated_candy.cols, annotated_candy.rows);
		cv::rectangle(annotated_candy, r, cv::Scalar(0, 0, 255), annotation_thickness, cv::LINE_AA);
	}

	if (flags & Flags::kDrawContour)
	{
		for (auto & contour : differences_contours)
		{
			cv::polylines(annotated_candy, contour, true, annotation_colour, annotation_thickness, cv::LINE_AA);
		}
	}

	if (flags & Flags::kDrawRectangle)
	{
		for (auto & rr : vz::convert_contours_to_rotated_rects(differences_contours))
		{
			cv::Point2f points[4];
			rr.points(points);
			for (int i = 0; i < 4; i++)
			{
				cv::line(annotated_candy, points[i], points[(i + 1) % 4], annotation_colour, annotation_thickness, cv::LINE_AA);
			}
		}
	}

	return annotated_candy;
}

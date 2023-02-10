//==============================================================================
//
//  OvenMediaEngine
//
//  Created by Hyunjun Jang
//  Copyright (c) 2020 AirenSoft. All rights reserved.
//
//==============================================================================
#include "record.h"

#include "common.h"

namespace serdes
{
	static void SetRecordStreamTrackIds(Json::Value &parent_object, const char *key, const std::shared_ptr<info::Record> &record, Optional optional)
	{
		CONVERTER_RETURN_IF(false, Json::arrayValue);

		for (auto &item : record->GetTrackIds())
		{
			object.append(item);
		}
	}

	static void SetRecordStreamTrackNames(Json::Value &parent_object, const char *key, const std::shared_ptr<info::Record> &record, Optional optional)
	{
		CONVERTER_RETURN_IF(false, Json::arrayValue);

		for (auto &item : record->GetTrackNames())
		{
			object.append(item.CStr());
		}
	}

	static void SetRecordStream(Json::Value &parent_object, const char *key, const std::shared_ptr<info::Record> &record, Optional optional)
	{
		CONVERTER_RETURN_IF(false, Json::objectValue);

		SetString(object, "name", record->GetStreamName(), Optional::False);
		SetRecordStreamTrackIds(object, "trackIds", record, Optional::False);
		SetRecordStreamTrackNames(object, "trackNames", record, Optional::False);
	}

	// -----------------------
	// Example of Record Start
	// -----------------------
	// 	{
	// 		"id": "<CustomId | AutoGeneratedId>",
	// 		"stream" : {
	// 			"name" : "<OutputStreamName>",
	// 			"trackIds" : [ 101, 102 ],
	// 			"trackNames" : [ "encodes_name_h264", "encodes_name_aac" ]
	// 		},
	//
	// 		"interval": 60000,
	// 		"schedule": "*/10 * *",
	//	    "segmentationRule": "continuity | discontinuity",
	//
    // 		"filePath": "webrtc_record_${Sequence}.mp4",
    // 		"infoPath": "file.xml",
    // 		"metadata": {
    //     		"key1": "value",
    //     		"key2": "value",
    //     		"key3": "value"
	//		}
	// 	}
	//
	//  [Optional] Split Recording
	// 		- Use only one of the 'interval' and 'schedule' for split recording.
	// 		- 'interval' is separated based on accumulated recording time. 
	// 		- 'schedule' sets the time of splitting a file. Same as crontab pattern
	//  	- 'segmentationRule' sets the start timestamp of the segmented files. 'continuity' option increases the timestamp of the split file, 
	//		   and the 'discontinuity' is initialized to 0.
	//  * 'stream.tracks' parameter will be deprecated. Use the 'stream.trackIds' parameter instead.
	//
	//
	// ----------------------
	// Example of Record Stop
	// ----------------------
	// 	{
	// 		"id": "<CustomId | AutoGeneratedId>"
	// 	}
	std::shared_ptr<info::Record> RecordFromJson(const Json::Value &json_body)
	{
		auto record = std::make_shared<info::Record>();

		// <Required>
		auto json_id = json_body["id"];
		if (json_id.empty() == false && json_id.isString() == true)
		{
			record->SetId(json_id.asString().c_str());
		}

		// <Required>
		auto json_stream = json_body["stream"];
		if (json_stream.empty() == false || json_stream.isObject() == true)
		{
			// <Required>
			auto json_stream_name = json_stream["name"];
			if(json_stream_name.empty() == false && json_stream_name.isString() == true)
			{
				record->SetStreamName(json_stream_name.asString().c_str());
			}

			// <Optional>
			auto json_stream_track_ids = json_stream["trackIds"];
			if(json_stream_track_ids.empty() == true)
			{
				json_stream_track_ids = json_stream["tracks"];
			}
			if (json_stream_track_ids.empty() == false && json_stream_track_ids.isArray() == true)
			{
				for (uint32_t i = 0; i < json_stream_track_ids.size(); i++)
				{
					if (json_stream_track_ids[i].isInt())
					{
						record->AddTrackId(json_stream_track_ids[i].asInt());
					}
				}
			}

			// <Optional>
			auto json_stream_track_names = json_stream["trackNames"];
			if (json_stream_track_names.empty() == false && json_stream_track_names.isArray() == true)
			{
				for (uint32_t i = 0; i < json_stream_track_names.size(); i++)
				{
					if (json_stream_track_names[i].isString())
					{
						record->AddTrackName(json_stream_track_names[i].asString().c_str());
					}
				}
			}
		}

		// <Optional>
		auto json_file_path = json_body["filePath"];
		if (json_file_path.empty() == false && json_file_path.isString() == true)
		{
			record->SetFilePath(json_file_path.asString().c_str());
		}

		// <Optional>
		auto json_info_path = json_body["infoPath"];
		if (json_info_path.empty() == false && json_info_path.isString() == true)
		{
			record->SetInfoPath(json_info_path.asString().c_str());
		}

		// <Optional>
		auto json_interval = json_body["interval"];
		if (json_interval.empty() == false && json_interval.isInt() == true)
		{
			record->SetInterval(json_interval.asInt());
		}
		else
		{
			record->SetInterval(0);
		}

		// <Optional>
		auto json_schedule = json_body["schedule"];
		if (json_schedule.empty() == false && json_schedule.isString() == true)
		{
			record->SetSchedule(json_schedule.asString().c_str());
		}
		else
		{
			record->SetSchedule("");
		}

		// <Optional>
		auto json_metadata = json_body["metadata"];
		if (json_metadata.empty() == false && json_metadata.isString() == true)
		{
			record->SetMetadata(json_metadata.asString().c_str());
		}

		// <Optional>
		record->SetSegmentationRule("discontinuity");
		auto json_segmentation_rule = json_body["segmentationRule"];
		if (json_segmentation_rule.empty() == false && json_segmentation_rule.isString() == true)
		{
			if (json_segmentation_rule.asString().compare("discontinuity") == 0 ||
				json_segmentation_rule.asString().compare("continuity") == 0)
			{
				record->SetSegmentationRule(json_segmentation_rule.asString().c_str());
			}
		}

		return record;
	}

	Json::Value JsonFromRecord(const std::shared_ptr<info::Record> &record)
	{
		Json::Value response(Json::ValueType::objectValue);

		SetString(response, "state", record->GetStateString(), Optional::False);

		SetString(response, "id", record->GetId(), Optional::False);

		SetString(response, "metadata", record->GetMetadata(), Optional::True);

		SetString(response, "vhost", record->GetVhost(), Optional::False);

		SetString(response, "app", record->GetApplication(), Optional::False);

		SetRecordStream(response, "stream", record, Optional::True);

		SetString(response, "filePath", record->GetFilePath(), Optional::False);

		SetString(response, "outputFilePath", record->GetOutputFilePath(), Optional::False);

		SetString(response, "infoPath", record->GetInfoPath(), Optional::False);

		SetString(response, "outputInfoPath", record->GetOutputInfoPath(), Optional::False);

		if (record->GetInterval() > 0)
		{
			SetInt(response, "interval", record->GetInterval());
		}

		if (record->GetSchedule().IsEmpty() == false)
		{
			SetString(response, "schedule", record->GetSchedule(), Optional::True);
		}

		if (record->GetSegmentationRule().IsEmpty() == false)
		{
			SetString(response, "segmentationRule", record->GetSegmentationRule(), Optional::True);
		}

		if (record->GetCreatedTime() != std::chrono::system_clock::from_time_t(0))
		{
			SetTimestamp(response, "createdTime", record->GetCreatedTime());
		}

		if (record->GetState() == info::Record::RecordState::Recording ||
			record->GetState() == info::Record::RecordState::Stopping ||
			record->GetState() == info::Record::RecordState::Stopped ||
			record->GetState() == info::Record::RecordState::Error)
		{
			if (record->GetRecordBytes() > 0)
				SetInt64(response, "recordBytes", record->GetRecordBytes());

			if (record->GetRecordTime() > 0)
				SetInt64(response, "recordTime", record->GetRecordTime());

			if (record->GetRecordTotalBytes() > 0)
				SetInt64(response, "totalRecordBytes", record->GetRecordTotalBytes());

			if (record->GetRecordTotalTime() > 0)
				SetInt64(response, "totalRecordTime", record->GetRecordTotalTime());

			if (record->GetSequence() > 0)
				SetInt(response, "sequence", record->GetSequence());

			if (record->GetRecordStartTime() != std::chrono::system_clock::from_time_t(0))
			{
				SetTimestamp(response, "startTime", record->GetRecordStartTime());
			}
		}

		if (record->GetState() == info::Record::RecordState::Stopped ||
			record->GetState() == info::Record::RecordState::Error)
		{
			if (record->GetRecordStopTime() != std::chrono::system_clock::from_time_t(0))
			{
				SetTimestamp(response, "finishTime", record->GetRecordStopTime());
			}
		}
		return response;
	}

}  // namespace serdes
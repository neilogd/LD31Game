#include "Math/MaCPUVec3d.h"
#include "Math/MaMat4d.h"
#include "Math/MaMat3d.h"
#include "Math/MaQuat.h"
#include "Math/MaPlane.h"
#include "Math/MaAABB.h"
#include "Math/MaCPUVec4d.h"
#include "Math/MaCPUVec2d.h"
#include "System/Scene/Rendering/ScnRenderTarget.h"
#include "System/Scene/Rendering/ScnTexture.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnRenderPipeline.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnTextureAtlas.h"
#include "System/Scene/Rendering/ScnDebugRenderComponent.h"
#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnSpriteComponent.h"
#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/Rendering/ScnFont.h"
#include "System/Scene/Rendering/ScnLightComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnParticleSystemComponent.h"
#include "System/Scene/Rendering/ScnModel.h"
#include "System/Scene/Rendering/ScnShader.h"
#include "System/Scene/ScnSpatialComponent.h"
#include "System/Scene/Animation/ScnAnimationPose.h"
#include "System/Scene/Animation/ScnAnimationTreeBlendNode.h"
#include "System/Scene/Animation/ScnAnimation.h"
#include "System/Scene/Animation/ScnAnimationTreeNode.h"
#include "System/Scene/Animation/ScnAnimationComponent.h"
#include "System/Scene/Animation/ScnAnimationTreeTrackNode.h"
#include "System/Scene/Sound/ScnSound.h"
#include "System/Scene/Sound/ScnSoundListenerComponent.h"
#include "System/Scene/Sound/ScnSoundEmitter.h"
#include "System/Scene/Physics/ScnPhysicsCollisionShape.h"
#include "System/Scene/Physics/ScnPhysicsWorldComponent.h"
#include "System/Scene/Physics/ScnPhysicsRigidBodyComponent.h"
#include "System/Scene/Physics/ScnPhysicsBoxCollisionShape.h"
#include "System/Scene/ScnComponent.h"
#include "System/Scene/Import/ScnEntityImport.h"
#include "System/Scene/Import/ScnTextureImport.h"
#include "System/Scene/Import/ScnAnimationImport.h"
#include "System/Scene/Import/ScnModelImport.h"
#include "System/Scene/Import/ScnComponentImport.h"
#include "System/Scene/Import/ScnSoundImport.h"
#include "System/Scene/Import/ScnShaderImport.h"
#include "System/Scene/Import/ScnFontImport.h"
#include "System/Scene/Import/ScnMaterialImport.h"
#include "System/Scene/ScnEntity.h"
#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsCoreImpl.h"
#include "System/Renderer/RsRenderState.h"
#include "System/File/FsTypes.h"
#include "System/Sound/SsCore.h"
#include "System/Sound/SoLoud/SsCoreImplSoLoud.h"
#include "System/SysSystem.h"
#include "System/SysKernel.h"
#include "System/Content/CsPackageImporter.h"
#include "System/Content/CsPackage.h"
#include "System/Content/CsTypes.h"
#include "System/Content/CsResource.h"
#include "System/Content/CsResourceImporter.h"
#include "GaWeaponComponent.h"
#include "GaTestModelComponent.h"
#include "GaCameraComponent.h"
#include "GaRobotComponent.h"
#include "GaWorldComponent.h"
#include "GaTestSelectionComponent.h"
void AutoGenRegisterReflection()
{
	MaCPUVec3d::StaticRegisterClass();
	MaMat4d::StaticRegisterClass();
	MaMat3d::StaticRegisterClass();
	MaQuat::StaticRegisterClass();
	MaPlane::StaticRegisterClass();
	MaAABB::StaticRegisterClass();
	MaCPUVec4d::StaticRegisterClass();
	MaCPUVec2d::StaticRegisterClass();
	ScnRenderTarget::StaticRegisterClass();
	ScnTexture::StaticRegisterClass();
	ScnRenderableComponent::StaticRegisterClass();
	ScnRenderPipeline::StaticRegisterClass();
	ScnViewComponent::StaticRegisterClass();
	ScnTextureAtlas::StaticRegisterClass();
	ScnDebugRenderComponent::StaticRegisterClass();
	ScnShaderViewUniformBlockData::StaticRegisterClass();
	ScnShaderLightUniformBlockData::StaticRegisterClass();
	ScnShaderObjectUniformBlockData::StaticRegisterClass();
	ScnShaderBoneUniformBlockData::StaticRegisterClass();
	ScnShaderAlphaTestUniformBlockData::StaticRegisterClass();
	ScnSpriteComponent::StaticRegisterClass();
	ScnCanvasComponent::StaticRegisterClass();
	ScnFont::StaticRegisterClass();
	ScnFontDrawParams::StaticRegisterClass();
	ScnFontComponent::StaticRegisterClass();
	ScnLightComponent::StaticRegisterClass();
	ScnMaterial::StaticRegisterClass();
	ScnMaterialComponent::StaticRegisterClass();
	ScnParticleSystemComponent::StaticRegisterClass();
	ScnModel::StaticRegisterClass();
	ScnModelComponent::StaticRegisterClass();
	ScnShader::StaticRegisterClass();
	ScnSpatialComponent::StaticRegisterClass();
	ScnAnimationPose::StaticRegisterClass();
	ScnAnimationTreeBlendNode::StaticRegisterClass();
	ScnAnimation::StaticRegisterClass();
	ScnAnimationTreeNode::StaticRegisterClass();
	ScnAnimationComponent::StaticRegisterClass();
	ScnAnimationTreeTrackNode::StaticRegisterClass();
	ScnSound::StaticRegisterClass();
	ScnSoundListenerComponent::StaticRegisterClass();
	ScnSoundEmitterComponent::StaticRegisterClass();
	ScnPhysicsCollisionShape::StaticRegisterClass();
	ScnPhysicsWorldComponent::StaticRegisterClass();
	ScnPhysicsRigidBodyComponent::StaticRegisterClass();
	ScnPhysicsBoxCollisionShape::StaticRegisterClass();
	ScnComponentAttribute::StaticRegisterClass();
	ScnComponent::StaticRegisterClass();
	ScnEntityImport::StaticRegisterClass();
	ScnTextureImport::StaticRegisterClass();
	ScnAnimationImport::StaticRegisterClass();
	ScnModelImport::StaticRegisterClass();
	ScnComponentImport::StaticRegisterClass();
	ScnSoundImport::StaticRegisterClass();
	ScnShaderImport::StaticRegisterClass();
	ScnFontImport::StaticRegisterClass();
	ScnMaterialImport::StaticRegisterClass();
	ScnEntity::StaticRegisterClass();
	RsColour::StaticRegisterClass();
	RsCore::StaticRegisterClass();
	RsSamplerStateDesc::StaticRegisterClass();
	RsCoreImpl::StaticRegisterClass();
	RsRenderTargetBlendState::StaticRegisterClass();
	RsBlendState::StaticRegisterClass();
	RsStencilFaceState::StaticRegisterClass();
	RsDepthStencilState::StaticRegisterClass();
	RsRasteriserState::StaticRegisterClass();
	RsRenderStateDesc::StaticRegisterClass();
	FsTimestamp::StaticRegisterClass();
	FsStats::StaticRegisterClass();
	SsCore::StaticRegisterClass();
	SsCoreImplSoLoud::StaticRegisterClass();
	SysSystem::StaticRegisterClass();
	SysKernel::StaticRegisterClass();
	CsPackageDependencies::StaticRegisterClass();
	CsPackage::StaticRegisterClass();
	CsDependency::StaticRegisterClass();
	CsResource::StaticRegisterClass();
	CsResourceImporterAttribute::StaticRegisterClass();
	CsResourceImporter::StaticRegisterClass();
	GaWeaponComponent::StaticRegisterClass();
	GaTestModelComponent::StaticRegisterClass();
	GaCameraComponent::StaticRegisterClass();
	GaRobotCommandEntry::StaticRegisterClass();
	GaRobotOperation::StaticRegisterClass();
	GaRobotComponent::StaticRegisterClass();
	GaWorldComponent::StaticRegisterClass();
	GaTestSelectionComponent::StaticRegisterClass();
}

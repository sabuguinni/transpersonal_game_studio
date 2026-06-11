#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Crowd_MassEntitySubsystem.h"
#include "Crowd_VisualComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_VisualComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_VisualComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Visual configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Visual")
    UStaticMesh* AgentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Visual")
    UMaterialInterface* AgentMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Visual")
    FVector AgentScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Visual")
    bool bEnableVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Visual")
    float UpdateFrequency;

    // Visual representation functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Visual")
    void InitializeVisualization();

    UFUNCTION(BlueprintCallable, Category = "Crowd Visual")
    void UpdateVisualization();

    UFUNCTION(BlueprintCallable, Category = "Crowd Visual")
    void ClearVisualization();

    UFUNCTION(BlueprintCallable, Category = "Crowd Visual")
    void SetAgentMesh(UStaticMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Crowd Visual")
    void SetAgentMaterial(UMaterialInterface* NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Crowd Visual")
    int32 GetVisibleAgentCount() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Visual")
    UInstancedStaticMeshComponent* InstancedMeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Visual")
    UCrowd_MassEntitySubsystem* CrowdSubsystem;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Visual")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Visual")
    TArray<FTransform> CachedTransforms;

private:
    void CreateInstancedMeshComponent();
    void UpdateInstanceTransforms();
    FTransform AgentDataToTransform(const FCrowd_AgentData& AgentData);
    void OptimizeInstanceRendering();
};
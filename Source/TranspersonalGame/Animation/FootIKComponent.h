#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootIKComponent.generated.h"

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
	FVector LeftFootLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
	FVector RightFootLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
	FRotator LeftFootRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
	FRotator RightFootRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
	float LeftFootOffset = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
	float RightFootOffset = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
	float PelvisOffset = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
	bool bIsOnUnevenTerrain = false;
};

UCLASS(ClassGroup = (Animation), meta = (BlueprintSpawnableComponent), DisplayName = "Foot IK Component")
class TRANSPERSONALGAME_API UFootIKComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFootIKComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
	FAnim_FootIKData GetFootIKData() const;

	UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
	void UpdateFootIK(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	float TraceDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	float InterpSpeed = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	float MaxFootOffset = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	FName LeftFootBoneName = TEXT("foot_l");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
	FName RightFootBoneName = TEXT("foot_r");

private:
	FAnim_FootIKData CurrentIKData;

	bool TraceForFoot(const FVector& FootLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
	float CalculatePelvisOffset(float LeftOffset, float RightOffset) const;
};

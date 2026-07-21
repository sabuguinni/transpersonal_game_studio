#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Arch_CretaceousFoundationSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_FoundationType : uint8
{
	CircularStone UMETA(DisplayName = "Circular Stone Foundation"),
	RectangularStone UMETA(DisplayName = "Rectangular Stone Foundation"),
	NaturalRock UMETA(DisplayName = "Natural Rock Foundation"),
	ElevatedPlatform UMETA(DisplayName = "Elevated Stone Platform"),
	CaveEntrance UMETA(DisplayName = "Cave Entrance Foundation"),
	CliffLedge UMETA(DisplayName = "Cliff Ledge Foundation")
};

USTRUCT(BlueprintType)
struct FArch_FoundationProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
	EArch_FoundationType FoundationType = EArch_FoundationType::CircularStone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation", meta = (ClampMin = "2.0", ClampMax = "20.0"))
	float Diameter = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WeatheringLevel = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MossGrowth = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
	bool bIsPartiallyBuried = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
	bool bHasWeatherProtection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ElevationHeight = 0.0f;

	FArch_FoundationProperties()
	{
		FoundationType = EArch_FoundationType::CircularStone;
		Diameter = 4.0f;
		WeatheringLevel = 0.3f;
		MossGrowth = 0.4f;
		bIsPartiallyBuried = true;
		bHasWeatherProtection = false;
		ElevationHeight = 0.0f;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_CretaceousFoundationSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UArch_CretaceousFoundationSystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation Properties")
	FArch_FoundationProperties FoundationConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foundation Components")
	class UStaticMeshComponent* FoundationMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foundation Components")
	class UBoxComponent* WeatherProtectionZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foundation Components")
	class USphereComponent* FoundationBounds;

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	void InitializeFoundation(const FArch_FoundationProperties& Properties);

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	void UpdateFoundationMesh();

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	void SetWeatheringLevel(float NewWeatheringLevel);

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	void SetMossGrowth(float NewMossGrowth);

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	bool IsLocationSuitableForFoundation(const FVector& Location, float RequiredRadius = 3.0f);

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	FVector GetOptimalFoundationLocation(const FVector& SearchCenter, float SearchRadius = 10.0f);

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	bool CanSupportStructure(float StructureWeight, float StructureRadius);

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	TArray<FVector> GetFoundationAnchorPoints();

	UFUNCTION(BlueprintCallable, Category = "Foundation System")
	void ApplyWeatheringEffect(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Foundation System")
	bool IsPlayerInWeatherProtectionZone() const;

	UFUNCTION(BlueprintPure, Category = "Foundation System")
	float GetFoundationStability() const;

	UFUNCTION(BlueprintPure, Category = "Foundation System")
	EArch_FoundationType GetFoundationType() const { return FoundationConfig.FoundationType; }

protected:
	UPROPERTY()
	float CurrentStability;

	UPROPERTY()
	float WeatheringAccumulation;

	UPROPERTY()
	TArray<FVector> AnchorPoints;

	void GenerateAnchorPoints();
	void UpdateWeatherProtectionZone();
	void CalculateStability();
	UStaticMesh* GetFoundationMeshForType(EArch_FoundationType Type);
	void ApplyMaterialProperties();

private:
	bool bIsInitialized;
	float LastWeatheringUpdate;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_FoundationActor : public AActor
{
	GENERATED_BODY()

public:
	AArch_FoundationActor();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foundation")
	UArch_CretaceousFoundationSystem* FoundationSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foundation")
	UStaticMeshComponent* FoundationMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foundation")
	USceneComponent* RootSceneComponent;

	UFUNCTION(BlueprintCallable, Category = "Foundation")
	void SetupFoundation(EArch_FoundationType Type, float Diameter, float WeatheringLevel = 0.3f);

	UFUNCTION(BlueprintCallable, Category = "Foundation")
	bool ValidateFoundationPlacement();

	UFUNCTION(BlueprintImplementableEvent, Category = "Foundation")
	void OnFoundationInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "Foundation")
	void OnWeatheringChanged(float NewWeatheringLevel);
};
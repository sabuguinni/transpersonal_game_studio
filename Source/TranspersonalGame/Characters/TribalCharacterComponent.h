#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TribalCharacterComponent.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"), 
    Elder       UMETA(DisplayName = "Elder"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Crafter     UMETA(DisplayName = "Crafter")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance") 
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float ScarIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFacePaint = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTribalCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTribalCharacterComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EChar_TribalRole TribalRole = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FString CharacterName = TEXT("Tribal Member");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    float ExperienceLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    TArray<FString> Skills;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal Character")
    UMaterialInstanceDynamic* DynamicMaterial;

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ApplyTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    FString GetRoleDescription() const;

private:
    void InitializeDefaultSkills();
    void UpdateMaterialParameters();
    USkeletalMeshComponent* GetCharacterMesh() const;
};
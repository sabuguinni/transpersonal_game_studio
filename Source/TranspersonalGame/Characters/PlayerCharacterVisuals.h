#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "PlayerCharacterVisuals.generated.h"

UENUM(BlueprintType)
enum class EChar_PlayerBodyType : uint8
{
    Athletic,
    Lean,
    Muscular,
    Average
};

UENUM(BlueprintType)
enum class EChar_PlayerSkinTone : uint8
{
    Light,
    Medium,
    Tan,
    Dark,
    VeryDark
};

UENUM(BlueprintType)
enum class EChar_PlayerHairStyle : uint8
{
    Short,
    Medium,
    Long,
    Braided,
    Shaved
};

USTRUCT(BlueprintType)
struct FChar_PlayerAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_PlayerBodyType BodyType = EChar_PlayerBodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_PlayerSkinTone SkinTone = EChar_PlayerSkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_PlayerHairStyle HairStyle = EChar_PlayerHairStyle::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Weight = 1.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPlayerCharacterVisuals : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerCharacterVisuals();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void ApplyPlayerAppearance(const FChar_PlayerAppearance& Appearance);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetPlayerMesh(USkeletalMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetPlayerMaterial(UMaterialInterface* NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void RandomizePlayerAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    FChar_PlayerAppearance GetCurrentAppearance() const { return CurrentAppearance; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visuals")
    FChar_PlayerAppearance CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visuals")
    TArray<TSoftObjectPtr<USkeletalMesh>> PlayerMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visuals")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visuals")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

private:
    void UpdateCharacterMesh();
    void UpdateCharacterMaterials();
    UMaterialInterface* GetSkinMaterialForTone(EChar_PlayerSkinTone SkinTone);
    USkeletalMesh* GetMeshForBodyType(EChar_PlayerBodyType BodyType);
};
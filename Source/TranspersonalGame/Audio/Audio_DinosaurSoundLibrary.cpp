#include "Audio_DinosaurSoundLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

UAudio_DinosaurSoundLibrary::UAudio_DinosaurSoundLibrary()
{
    GlobalDinosaurVolume = 1.0f;
    bEnableTerritorialAudio = true;
    
    InitializeDefaultProfiles();
}

void UAudio_DinosaurSoundLibrary::InitializeDefaultProfiles()
{
    SpeciesProfiles.Empty();
    
    // T-Rex Profile - Massive, territorial predator
    FAudio_DinosaurSpeciesProfile TRexProfile;
    TRexProfile.Species = EAudio_DinosaurSpecies::TRex;
    TRexProfile.SpeciesVolumeMultiplier = 1.5f;
    TRexProfile.TerritorialRange = 3000.0f;
    
    // T-Rex Roar
    FAudio_DinosaurSoundData TRexRoar;
    TRexRoar.SoundType = EAudio_DinosaurSoundType::Roar;
    TRexRoar.BaseVolume = 1.0f;
    TRexRoar.PitchVariation = 0.15f;
    TRexRoar.MaxDistance = 8000.0f;
    TRexRoar.Cooldown = 5.0f;
    TRexProfile.SoundLibrary.Add(TRexRoar);
    
    // T-Rex Footstep
    FAudio_DinosaurSoundData TRexFootstep;
    TRexFootstep.SoundType = EAudio_DinosaurSoundType::Footstep;
    TRexFootstep.BaseVolume = 0.8f;
    TRexFootstep.PitchVariation = 0.1f;
    TRexFootstep.MaxDistance = 2000.0f;
    TRexFootstep.Cooldown = 0.5f;
    TRexProfile.SoundLibrary.Add(TRexFootstep);
    
    SpeciesProfiles.Add(TRexProfile);
    
    // Raptor Profile - Pack hunter, agile
    FAudio_DinosaurSpeciesProfile RaptorProfile;
    RaptorProfile.Species = EAudio_DinosaurSpecies::Raptor;
    RaptorProfile.SpeciesVolumeMultiplier = 0.7f;
    RaptorProfile.TerritorialRange = 1500.0f;
    
    // Raptor Screech
    FAudio_DinosaurSoundData RaptorScreech;
    RaptorScreech.SoundType = EAudio_DinosaurSoundType::Roar;
    RaptorScreech.BaseVolume = 0.6f;
    RaptorScreech.PitchVariation = 0.3f;
    RaptorScreech.MaxDistance = 3000.0f;
    RaptorScreech.Cooldown = 3.0f;
    RaptorProfile.SoundLibrary.Add(RaptorScreech);
    
    // Raptor Footstep
    FAudio_DinosaurSoundData RaptorFootstep;
    RaptorFootstep.SoundType = EAudio_DinosaurSoundType::Footstep;
    RaptorFootstep.BaseVolume = 0.3f;
    RaptorFootstep.PitchVariation = 0.2f;
    RaptorFootstep.MaxDistance = 800.0f;
    RaptorFootstep.Cooldown = 0.3f;
    RaptorProfile.SoundLibrary.Add(RaptorFootstep);
    
    SpeciesProfiles.Add(RaptorProfile);
    
    // Brachiosaurus Profile - Gentle giant
    FAudio_DinosaurSpeciesProfile BrachiosaurusProfile;
    BrachiosaurusProfile.Species = EAudio_DinosaurSpecies::Brachiosaurus;
    BrachiosaurusProfile.SpeciesVolumeMultiplier = 1.2f;
    BrachiosaurusProfile.TerritorialRange = 2500.0f;
    
    // Brachiosaurus Low Call
    FAudio_DinosaurSoundData BrachiosaurusCall;
    BrachiosaurusCall.SoundType = EAudio_DinosaurSoundType::Roar;
    BrachiosaurusCall.BaseVolume = 0.9f;
    BrachiosaurusCall.PitchVariation = 0.1f;
    BrachiosaurusCall.MaxDistance = 6000.0f;
    BrachiosaurusCall.Cooldown = 8.0f;
    BrachiosaurusProfile.SoundLibrary.Add(BrachiosaurusCall);
    
    // Brachiosaurus Heavy Footstep
    FAudio_DinosaurSoundData BrachiosaurusFootstep;
    BrachiosaurusFootstep.SoundType = EAudio_DinosaurSoundType::Footstep;
    BrachiosaurusFootstep.BaseVolume = 1.0f;
    BrachiosaurusFootstep.PitchVariation = 0.05f;
    BrachiosaurusFootstep.MaxDistance = 1500.0f;
    BrachiosaurusFootstep.Cooldown = 1.0f;
    BrachiosaurusProfile.SoundLibrary.Add(BrachiosaurusFootstep);
    
    SpeciesProfiles.Add(BrachiosaurusProfile);
}

bool UAudio_DinosaurSoundLibrary::PlayDinosaurSound(EAudio_DinosaurSpecies Species, EAudio_DinosaurSoundType SoundType, 
                                                   FVector Location, AActor* SourceActor)
{
    FAudio_DinosaurSpeciesProfile* Profile = GetSpeciesProfile(Species);
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("No audio profile found for dinosaur species: %d"), (int32)Species);
        return false;
    }
    
    FAudio_DinosaurSoundData* SoundData = GetSoundData(Profile, SoundType);
    if (!SoundData)
    {
        UE_LOG(LogTemp, Warning, TEXT("No sound data found for species %d, sound type %d"), (int32)Species, (int32)SoundType);
        return false;
    }
    
    // Check cooldown
    FString CooldownKey = FString::Printf(TEXT("%d_%d_%s"), 
        (int32)Species, 
        (int32)SoundType, 
        SourceActor ? *SourceActor->GetName() : TEXT("World"));
    
    if (IsOnCooldown(CooldownKey, SoundData->Cooldown))
    {
        return false;
    }
    
    // Select random sound variation
    USoundCue* SelectedSound = SelectRandomSoundVariation(SoundData->SoundVariations);
    if (!SelectedSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("No sound variations available for species %d, sound type %d"), (int32)Species, (int32)SoundType);
        return false;
    }
    
    // Calculate final volume
    float FinalVolume = SoundData->BaseVolume * Profile->SpeciesVolumeMultiplier * GlobalDinosaurVolume;
    
    // Calculate pitch variation
    float PitchMultiplier = 1.0f + FMath::RandRange(-SoundData->PitchVariation, SoundData->PitchVariation);
    
    // Play sound at location
    UGameplayStatics::PlaySoundAtLocation(
        this,
        SelectedSound,
        Location,
        FinalVolume,
        PitchMultiplier,
        0.0f, // Start time
        nullptr, // Sound attenuation override
        nullptr, // Sound concurrency override
        SourceActor
    );
    
    // Set cooldown
    SoundCooldowns.Add(CooldownKey, GetWorld()->GetTimeSeconds());
    
    UE_LOG(LogTemp, Log, TEXT("Played dinosaur sound: Species %d, Type %d at location %s"), 
        (int32)Species, (int32)SoundType, *Location.ToString());
    
    return true;
}

UAudioComponent* UAudio_DinosaurSoundLibrary::PlayDinosaurSoundWithComponent(EAudio_DinosaurSpecies Species, 
                                                                           EAudio_DinosaurSoundType SoundType,
                                                                           USceneComponent* AttachComponent)
{
    if (!AttachComponent)
    {
        return nullptr;
    }
    
    FAudio_DinosaurSpeciesProfile* Profile = GetSpeciesProfile(Species);
    if (!Profile)
    {
        return nullptr;
    }
    
    FAudio_DinosaurSoundData* SoundData = GetSoundData(Profile, SoundType);
    if (!SoundData)
    {
        return nullptr;
    }
    
    USoundCue* SelectedSound = SelectRandomSoundVariation(SoundData->SoundVariations);
    if (!SelectedSound)
    {
        return nullptr;
    }
    
    // Create audio component
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(
        SelectedSound,
        AttachComponent,
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        false, // Stop when owner destroyed
        SoundData->BaseVolume * Profile->SpeciesVolumeMultiplier * GlobalDinosaurVolume,
        1.0f + FMath::RandRange(-SoundData->PitchVariation, SoundData->PitchVariation),
        0.0f, // Start time
        nullptr, // Attenuation override
        nullptr, // Concurrency override
        false // Auto destroy
    );
    
    return AudioComp;
}

void UAudio_DinosaurSoundLibrary::SetSpeciesVolumeMultiplier(EAudio_DinosaurSpecies Species, float VolumeMultiplier)
{
    FAudio_DinosaurSpeciesProfile* Profile = GetSpeciesProfile(Species);
    if (Profile)
    {
        Profile->SpeciesVolumeMultiplier = FMath::Clamp(VolumeMultiplier, 0.0f, 2.0f);
    }
}

FAudio_DinosaurSpeciesProfile* UAudio_DinosaurSoundLibrary::GetSpeciesProfile(EAudio_DinosaurSpecies Species)
{
    for (FAudio_DinosaurSpeciesProfile& Profile : SpeciesProfiles)
    {
        if (Profile.Species == Species)
        {
            return &Profile;
        }
    }
    return nullptr;
}

TArray<USoundCue*> UAudio_DinosaurSoundLibrary::GetSoundVariations(EAudio_DinosaurSpecies Species, EAudio_DinosaurSoundType SoundType)
{
    TArray<USoundCue*> LoadedSounds;
    
    FAudio_DinosaurSpeciesProfile* Profile = GetSpeciesProfile(Species);
    if (!Profile)
    {
        return LoadedSounds;
    }
    
    FAudio_DinosaurSoundData* SoundData = GetSoundData(Profile, SoundType);
    if (!SoundData)
    {
        return LoadedSounds;
    }
    
    for (const TSoftObjectPtr<USoundCue>& SoundPtr : SoundData->SoundVariations)
    {
        if (USoundCue* LoadedSound = SoundPtr.LoadSynchronous())
        {
            LoadedSounds.Add(LoadedSound);
        }
    }
    
    return LoadedSounds;
}

FAudio_DinosaurSoundData* UAudio_DinosaurSoundLibrary::GetSoundData(FAudio_DinosaurSpeciesProfile* Profile, EAudio_DinosaurSoundType SoundType)
{
    if (!Profile)
    {
        return nullptr;
    }
    
    for (FAudio_DinosaurSoundData& SoundData : Profile->SoundLibrary)
    {
        if (SoundData.SoundType == SoundType)
        {
            return &SoundData;
        }
    }
    return nullptr;
}

USoundCue* UAudio_DinosaurSoundLibrary::SelectRandomSoundVariation(const TArray<TSoftObjectPtr<USoundCue>>& Variations)
{
    if (Variations.Num() == 0)
    {
        return nullptr;
    }
    
    int32 RandomIndex = FMath::RandRange(0, Variations.Num() - 1);
    return Variations[RandomIndex].LoadSynchronous();
}

bool UAudio_DinosaurSoundLibrary::IsOnCooldown(const FString& CooldownKey, float CooldownTime)
{
    if (!SoundCooldowns.Contains(CooldownKey))
    {
        return false;
    }
    
    float LastPlayTime = SoundCooldowns[CooldownKey];
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    return (CurrentTime - LastPlayTime) < CooldownTime;
}